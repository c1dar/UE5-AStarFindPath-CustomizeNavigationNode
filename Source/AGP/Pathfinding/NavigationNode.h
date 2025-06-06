// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavigationNode.generated.h"
UENUM(BlueprintType)
enum class EPointType : uint8
{
	Normal      UMETA(DisplayName = "Normal"), 
	Cover       UMETA(DisplayName = "Cover"), 
	SpawnPoint  UMETA(DisplayName = "Spawn Point"), 
	EscapePoint UMETA(DisplayName = "Escape Point") 
};
	
UCLASS()
class AGP_API ANavigationNode : public AActor
{
	GENERATED_BODY()

	friend class UPathfindingSubsystem;
	friend class SubSystemAI;
	
public:	
	// Sets default values for this actor's properties
	ANavigationNode();

	virtual bool ShouldTickIfViewportsOnly() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> ConnectedNodes;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* LocationComponent;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY(EditAnywhere)
	EPointType NodeType = EPointType::Normal;
};
