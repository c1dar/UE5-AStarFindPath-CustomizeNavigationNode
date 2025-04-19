// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SubSystemAI.generated.h"

/**
 * 
 */
class ANavigationNode;
UCLASS()
class AGP_API USubSystemAI : public UWorldSubsystem
{
	GENERATED_BODY()

	
public:
	void OnWorldBeginPlay(UWorld& InWorld);
	void SneakAway();
private:



};
