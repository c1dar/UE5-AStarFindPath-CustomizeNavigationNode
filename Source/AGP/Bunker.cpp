// Fill out your copyright notice in the Description page of Project Settings.


#include "Bunker.h"

// Sets default values
ABunker::ABunker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABunker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABunker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

