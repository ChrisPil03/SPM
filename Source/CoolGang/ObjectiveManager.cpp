// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveManager.h"

// Sets default values
AObjectiveManager::AObjectiveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AObjectiveManager::BeginPlay()
{
	Super::BeginPlay();

	if (!Portal || !ExtractionZone)
	{
		return;
	}
	Portal->SetActorScale3D(FVector::ZeroVector);
	ExtractionZone->SetActorEnableCollision(false);
}

void AObjectiveManager::RegisterCompletedObjective()
{
	CompletedObjectives++;

	UE_LOG(LogTemp, Warning, TEXT("Completed Objectives: %d"), CompletedObjectives);

	if (GetIsObjectivesCompleted())
	{
		ObjectivesCompleted();
	}
}

void AObjectiveManager::ObjectivesCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("Objectives Completed!"));

	if (!Portal || !ExtractionZone)
	{
		return;
	}
	Portal->SetActorScale3D(FVector::One());
	ExtractionZone->SetActorEnableCollision(true);
	
	// TODO: Start count down timer
}

bool AObjectiveManager::GetIsObjectivesCompleted() const
{
	return CompletedObjectives == ObjectivesInLevel.Num();
}