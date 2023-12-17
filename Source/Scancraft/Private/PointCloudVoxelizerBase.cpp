// Fill out your copyright notice in the Description page of Project Settings.


#include "PointCloudVoxelizerBase.h"
#include "VoxelImporters/VoxelMeshImporter.h"

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.inl"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelUtilities/VoxelExampleUtilities.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"
#include "VoxelMessages.h"

//#include "SDFGen/makelevelset3.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "Open3DUE5.h"

// Sets default values
APointCloudVoxelizerBase::APointCloudVoxelizerBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APointCloudVoxelizerBase::BeginPlay()
{
	Super::BeginPlay();
	FOpen3DUE5Module* Plugin = FModuleManager::GetModulePtr<FOpen3DUE5Module>("Open3DUE5");
	
	if (Plugin) Plugin->InitSensor();
	ReleaseSensorMemoryDelegate.BindUFunction(this, TEXT("ReleaseSensorMemory"));
	OnEndPlay.Add(ReleaseSensorMemoryDelegate);
}

bool VoxelSurroundedCheck(TVoxelSharedRef<FVoxelDataAssetData> Data, int X, int Y, int Z)
{
	int count = 1;
	for (auto i = -1; i < 2; ++i)
	{
		for (auto j = -1; j < 2; ++j)
		{
			for (auto k = -1; k < 2; ++k)
			{
				if (Data->GetValue(X + i, Y + k, Z + k, FVoxelValue::Special()) == FVoxelValue::Full())
				{
					count++;
				}
			}
		}
	}

	return count > 5;
}

void RemoveAdjutantVoxels(TVoxelSharedRef<FVoxelDataAssetData> Data, int X, int Y, int Z)
{
	for (auto i = -1; i < 2; ++i)
	{
		for (auto j = -1; j < 2; ++j)
		{
			for (auto k = -1; k < 2; ++k)
			{
				if (Data->IsValidIndex(X + i, Y + k, Z + k) && FMath::Abs(i) + FMath::Abs(j) + FMath::Abs(k) < 2)
				{
					Data->SetValue(X + i, Y + k, Z + k, FVoxelValue::Empty());
				}
			}
		}
	}
}

void CleanUpAloneVoxels(TVoxelSharedRef<FVoxelDataAssetData> Data, int IterNum)
{
	TArray<FVector> ToRemove = {};
	for (auto temp = 0; temp < IterNum; ++temp)
	{
		for (int32 X = 0; X < Data->GetSize().X; X++)
		{
			for (int32 Y = 0; Y < Data->GetSize().Y; Y++)
			{
				for (int32 Z = 0; Z < Data->GetSize().Z; Z++)
				{
					if (!VoxelSurroundedCheck(Data, X, Y, Z))
					{
						ToRemove.Add(FVector(X, Y, Z));
					}
				}
			}
		}

		for (auto& Coord : ToRemove)
		{
			Data->SetValue(Coord.X, Coord.Y, Coord.Z, FVoxelValue::Empty());
		}
	}
}

bool APointCloudVoxelizerBase::Voxelize(
	UObject* WorldContextObject,
	TArray<FVector3f> Points,
	FTransform Transform,
	bool bSubtractive,
	FVoxelMeshImporterSettingsBase Settings,
	UVoxelDataAsset*& Asset)
{
	//VOXEL_FUNCTION_COUNTER();

	if (Points.IsEmpty())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid Points Array"));
		return false;
	}

	if (bSubtractive)
	{
		Settings.DistanceDivisor *= -1;
	}

	FIntVector PositionOffset{ ForceInit };

	TArray<FIntVector> VoxelFilledArr = {};
	FIntVector RealSize = {};

	//FOpen3DUE5Module* Plugin = FModuleManager::GetModulePtr<FOpen3DUE5Module>("Open3DUE5");

	TArray<FVector3f> RealPoints = {};

	FVector TransformedPoint;
	for (auto& APoint : Points)
	{
		TransformedPoint = Transform.TransformPosition(FVector(APoint));

		RealPoints.Add(FVector3f(TransformedPoint));
	}

	FOpen3DUE5Module::VoxelizedArrFromPoints(RealPoints, (double)Settings.VoxelSize, VoxelFilledArr, RealSize);

	/**
	for (auto Voxel : VoxelFilledArr)
	{
		UE_LOG(LogClass, Warning, TEXT("Voxel at %s"), *Voxel.ToString())
	}
	//*/
	UE_LOG(LogClass, Warning, TEXT("Size: %s"), *RealSize.ToString())

	const auto Data = MakeVoxelShared<FVoxelDataAssetData>();

	auto RealSettings = FVoxelMeshImporterSettings(Settings);

	FBox Box(ForceInit);
	FVector Origin = Transform.TransformPosition(FVector::Zero());
	for (auto& Point : RealPoints)
	{
		const auto PointDouble = FVector(Point.X, Point.Y, Point.Z) - Origin;
		//Vertices.Add(NewVertex);
		Box += PointDouble;
	}
	//Box = Box.ExpandBy(Settings.VoxelSize);

	if (int64(RealSize.X) * int64(RealSize.Y) * int64(RealSize.Z) >= MAX_int32)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Converted assets would have more than 2B voxels! Abording"));
		return false;
	}
	if (RealSize.X * RealSize.Y * RealSize.Z == 0)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Size = 0! Abording"));
		return false;
	}

	auto OutOffset = FVoxelUtilities::RoundToInt(Box.Min/ RealSettings.VoxelSize);

	Data->SetSize(FIntVector(RealSize.X, RealSize.Y, RealSize.Z), false);

	for (int32 X = 0; X < RealSize.X; X++)
	{
		for (int32 Y = 0; Y < RealSize.Y; Y++)
		{
			for (int32 Z = 0; Z < RealSize.Z; Z++)
			{
				Data->SetValue(X, Y, Z, FVoxelValue::Empty());
			}
		}
	}

	for (auto& AVoxelLocation : VoxelFilledArr)
	{
		if (AVoxelLocation.X > RealSize.X || AVoxelLocation.Y > RealSize.Y || AVoxelLocation.Z > RealSize.Z)
		{
			UE_LOG(LogClass, Warning, TEXT("Something is wrong!"));
		}
		Data->SetValue(AVoxelLocation.X, AVoxelLocation.Y, AVoxelLocation.Z, FVoxelValue::Full());
	}

	/**/
	CleanUpAloneVoxels(Data, 2);
	//*/

	Asset = NewObject<UVoxelDataAsset>(GetTransientPackage());
	Asset->bSubtractiveAsset = bSubtractive;
	Asset->PositionOffset = OutOffset; // -RealSize / (2 * RealSettings.VoxelSize);
	Asset->SetData(Data);

	return true;
}

void APointCloudVoxelizerBase::ReleaseSensorMemory(AActor* DestroyedActor, EEndPlayReason::Type EndPlayReason)
{
	FOpen3DUE5Module* Plugin = FModuleManager::GetModulePtr<FOpen3DUE5Module>("Open3DUE5");

	Plugin->CleanUpSensorHPS();
}

TArray<FVector> APointCloudVoxelizerBase::GetOneFrameFromSensor(float VoxelSize)
{
	TArray<FVector> Points = {};
	TArray<FVector> CleanedUpPoints = {};

	FOpen3DUE5Module* Plugin = FModuleManager::GetModulePtr<FOpen3DUE5Module>("Open3DUE5");

	Plugin->GetSensorOneFrame(Points);

	FOpen3DUE5Module::CleanUpRawData(Points, VoxelSize, CleanedUpPoints);

	//UE_LOG(LogClass, Warning, TEXT("Cleaned Up! CleanedUpPoints First Value: %s"), *CleanedUpPoints[0].ToString());
	//Voxelize();

	return CleanedUpPoints;
}

// Called every frame
void APointCloudVoxelizerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

