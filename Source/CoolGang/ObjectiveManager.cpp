// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveManager.h"

#include "ObjectiveBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AObjectiveManager::AObjectiveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	ObjectivesInLevel.Empty();
	LastActivatedObjective = nullptr,
	Portal = nullptr;
	ExtractionZone = nullptr;
}

// Called when the game starts or when spawned
void AObjectiveManager::BeginPlay()
{
	Super::BeginPlay();
	FindObjectivesInLevel();

	if (!Portal || !ExtractionZone)
	{
		return;
	}
	Portal->SetActorScale3D(FVector::ZeroVector);
	ExtractionZone->SetActorEnableCollision(false);
}

void AObjectiveManager::ActivateRandomObjective()
{
	if (ObjectivesInLevel.IsEmpty())
	{
		return;
	}
	TArray<AObjectiveBase*> AvailableObjectives;
	for (AObjectiveBase* Objective : ObjectivesInLevel)
	{
		if (Objective && !Objective->GetIsActive() && !Objective->GetIsFailed())
		{
			AvailableObjectives.Add(Objective);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Available objectives: %d"), AvailableObjectives.Num());
	if (AvailableObjectives.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No available objectives to activate"));
		return;
	}
	if (AvailableObjectives.Num() > 1 && LastActivatedObjective)
	{
		AvailableObjectives.Remove(LastActivatedObjective);
	}
	int32 RandomIndex = FMath::RandRange(0, AvailableObjectives.Num() - 1);
	if (AObjectiveBase* SelectedObjective = AvailableObjectives[RandomIndex])
	{
		SelectedObjective->ResetObjective();
		SelectedObjective->SetIsActive(true);
		LastActivatedObjective = SelectedObjective;
	}
}

void AObjectiveManager::RegisterCompletedObjective()
{
	ActivateRandomObjective();
	CompletedObjectives++;

	UE_LOG(LogTemp, Warning, TEXT("Completed Objectives: %d"), CompletedObjectives);

	// if (GetIsObjectivesCompleted())
	// {
	// 	ObjectivesCompleted();
	// }
}

void AObjectiveManager::ResetAllObjectives()
{
	for (AObjectiveBase* Objective : ObjectivesInLevel)
	{
		Objective->ResetObjective();
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
}

bool AObjectiveManager::GetIsObjectivesCompleted() const
{
	return CompletedObjectives == ObjectivesInLevel.Num();
}

void AObjectiveManager::FindObjectivesInLevel()
{
	TArray<AActor*> FoundObjectives;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AObjectiveBase::StaticClass(), FoundObjectives);

	if (FoundObjectives.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectiveManager: No objectives found!"));
		return;
	}

	for (AActor* Actor : FoundObjectives)
	{
		if (AObjectiveBase* Objective = Cast<AObjectiveBase>(Actor))
		{
			ObjectivesInLevel.Add(Objective);
		}
	}
}
