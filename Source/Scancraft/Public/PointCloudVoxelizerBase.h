// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointCloudVoxelizerBase.generated.h"

UCLASS()
class SCANCRAFT_API APointCloudVoxelizerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APointCloudVoxelizerBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Voxelize Point Clouds")
	bool InitSensorStuff();

	UFUNCTION(BlueprintCallable, Category = "Voxelize Point Clouds", meta = (WorldContext = "WorldContextObject"))
	static bool Voxelize(
		UObject* WorldContextObject,
		TArray<FVector3f> Points,
		FTransform Transform,
		bool bSubtractive,
		FVoxelMeshImporterSettingsBase Settings,
		UVoxelDataAsset*& Asset);

	FScriptDelegate ReleaseSensorMemoryDelegate;

	UFUNCTION(BlueprintCallable, Category = "Voxelize Point Clouds")
	void ReleaseSensorMemory(AActor* DestroyedActor, EEndPlayReason::Type EndPlayReason);

	UFUNCTION(BlueprintCallable, Category = "Get 1 Frame Sensor Data", meta = (WorldContext = "WorldContextObject"))
	static TArray<FVector> GetOneFrameFromSensor(float VoxelSize);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
