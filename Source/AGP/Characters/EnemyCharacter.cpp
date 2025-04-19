// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "EngineUtils.h"
#include "HealthComponent.h"
#include "PlayerCharacter.h"
#include "AGP/Bunker.h"
#include "AGP/Pathfinding/PathfindingSubsystem.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InitialRotation = GetActorRotation();
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("Pawn Sensing Component");
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	PathfindingSubsystem = GetWorld()->GetSubsystem<UPathfindingSubsystem>();
	if (PathfindingSubsystem)
	{
		CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to find the PathfindingSubsystem"))
	}
	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSensedPawn);
	}
}

void AEnemyCharacter::MoveAlongPath()
{
	// Execute the path. Should be called each tick.

	// If the path is empty do nothing.
	if (CurrentPath.IsEmpty()) return;
	
	// 1. Move towards the current stage of the path.
	//		a. Calculate the direction from the current position to the target of the current stage of the path.
	FVector MovementDirection = CurrentPath[CurrentPath.Num()-1] - GetActorLocation();
	MovementDirection.Normalize();
	//		b. Apply movement in that direction.
	AddMovementInput(MovementDirection);
	// 2. Check if it is close to the current stage of the path then pop it off.
	if (FVector::Distance(GetActorLocation(), CurrentPath[CurrentPath.Num() - 1]) < PathfindingError)
	{
		CurrentPath.Pop();
	}
}

void AEnemyCharacter::TickPatrol()
{
	//UE_LOG(LogTemp, Display, TEXT("TickPatrol"))
	if (CurrentPath.IsEmpty())
	{
		CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
	}
	MoveAlongPath();
}

void AEnemyCharacter::TickEngage()
{
	//UE_LOG(LogTemp, Display, TEXT("TickEngage"))
	if (!SensedCharacter) return;
	
	if (CurrentPath.IsEmpty())
	{
		CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), SensedCharacter->GetActorLocation());
	}
	MoveAlongPath();
	Fire(SensedCharacter->GetActorLocation());
}

void AEnemyCharacter::TickEvade()
{
	//UE_LOG(LogTemp, Display, TEXT("TickEvade"))
	// Find the player and return if it can't find it.
	if (!SensedCharacter) return;

	if (CurrentPath.IsEmpty())
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		CurrentPath = PathfindingSubsystem->GetPathAway(GetActorLocation(), SensedCharacter->GetActorLocation());
	}
	MoveAlongPath();
}

void AEnemyCharacter::TickAway()
{
	if (CurrentPath.IsEmpty())
	{
		GetCharacterMovement()->MaxWalkSpeed = 400.0f;
		CurrentPath = PathfindingSubsystem->GetExitPath(GetActorLocation());
	}
	MoveAlongPath();
}

void AEnemyCharacter::TickIntoCover()
{
	if(CurrentPath.IsEmpty())
	{
		GetCharacterMovement()->MaxWalkSpeed = 700.0f;
		CurrentPath = PathfindingSubsystem->GetNearestCoverPath(GetActorLocation(),GetActorLocation());
	}
	MoveAlongPath();
	
}

void AEnemyCharacter::TickBack()
{
	if (CurrentPath.IsEmpty())
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.0f;
		CurrentPath = PathfindingSubsystem->GetSpawnPointPath(GetActorLocation());
	}
	MoveAlongPath();
}

void AEnemyCharacter::OnSensedPawn(APawn* SensedActor)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(SensedActor))
	{
		SensedCharacter = Player;
		UE_LOG(LogTemp, Display, TEXT("Sensed Player"))
	}
}

void AEnemyCharacter::UpdateSight()
{
	if (!SensedCharacter) return;
	if (PawnSensingComponent)
	{
		if (PawnSensingComponent && !PawnSensingComponent->HasLineOfSightTo(SensedCharacter))
		{
			SensedCharacter = nullptr;
			UE_LOG(LogTemp, Display, TEXT("Lost Player"))
		}
	}
}




// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSight();
	switch(CurrentState)
	{
	case EEnemyState::Evade:
		TickEvade();
		if(!SensedCharacter)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::SlipAway;
		}
		else
		{
			if(HealthComponent->GetCurrentHealthPercentage()<0.4f)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::LowHP;
			}
		}
		break;
		
	case EEnemyState::SlipAway:
		TickAway();
		if(SensedCharacter)
		{
			if(HealthComponent->GetCurrentHealthPercentage()==1.0f)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Controlled;
			}
			if(HealthComponent->GetCurrentHealthPercentage() >= 0.4f && HealthComponent->GetCurrentHealthPercentage() <= 0.9f)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Evade;
			}
			else if(HealthComponent->GetCurrentHealthPercentage()<0.4f)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::LowHP;
			}
			
		}
		break;
		
	case EEnemyState::LowHP:
		TickIntoCover();
		if(!SensedCharacter)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::SlipAway;
		}
		else
		{
			if(CurrentPath.IsEmpty())
			{
				FVector Target=	PathfindingSubsystem->FurthestSplinePoint(SensedCharacter->GetActorLocation());
				UE_LOG(LogTemp, Warning, TEXT("PointLocation: X = %f, Y = %f, Z = %f"), 
				Target.X, Target.Y, Target.Z);
				CurrentPath.Add(Target);
			}
		}
		
		break;
		
	case EEnemyState::Controlled:
		TickBack();
		if(!SensedCharacter)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::SlipAway;
		}
		if(SensedCharacter)
		{
			if(HealthComponent->GetCurrentHealthPercentage()<1.0f)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Evade;
			}
			
		}
		break;
	}
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

APlayerCharacter* AEnemyCharacter::FindPlayer() const
{
	APlayerCharacter* Player = nullptr;
	for (TActorIterator<APlayerCharacter> It(GetWorld()); It; ++It)
	{
		Player = *It;
		break;
	}
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to find the Player Character in the world."))
	}
	return Player;
}

