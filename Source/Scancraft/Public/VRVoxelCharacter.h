// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VRCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "VRVoxelCharacter.generated.h"


/** 
 * 
 */
UCLASS()
class SCANCRAFT_API AVRVoxelCharacter : public AVRCharacter
{

	GENERATED_BODY()

public:
	virtual void SetBase(UPrimitiveComponent* NewBase, FName BoneName, bool bNotifyActor) override
	{
		if (NewBase)
		{
			AActor* BaseOwner = NewBase->GetOwner();
			// LoadClass to not depend on the voxel module
			static UClass* VoxelWorldClass = LoadClass<UObject>(nullptr, TEXT("/Script/Voxel.VoxelWorld"));
			if (ensure(VoxelWorldClass) && BaseOwner && BaseOwner->IsA(VoxelWorldClass))
			{
				NewBase = Cast<UPrimitiveComponent>(BaseOwner->GetRootComponent());
				ensure(NewBase);
			}
		}
		Super::SetBase(NewBase, BoneName, bNotifyActor);
	}
	
};
