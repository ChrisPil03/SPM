// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveManagerSubsystem.h"

#include "ObjectiveBase.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
UObjectiveManagerSubsystem::UObjectiveManagerSubsystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	ObjectivesInLevel.Empty();
	LastCompletedObjective = nullptr;
}

// Called when the game starts or when spawned
void UObjectiveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UObjectiveManagerSubsystem::OnWorldInitialized);
	
}

void UObjectiveManagerSubsystem::OnWorldInitialized(const UWorld::FActorsInitializedParams& Params)
{
	FindObjectivesInLevel();
}

void UObjectiveManagerSubsystem::ActivateRandomObjective(float MalfunctionTimer, float MalfunctionInterval, float MalfunctionDamage)
{
	if (ObjectivesInLevel.IsEmpty())
	{
		return;
	}
	TArray<AObjectiveBase*> AvailableObjectives;
	for (AObjectiveBase* Objective : ObjectivesInLevel)
	{
		if (Objective && !Objective->GetIsActive())
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
	if (AvailableObjectives.Num() > 1 && LastCompletedObjective)
	{
		AvailableObjectives.Remove(LastCompletedObjective);
	}
	int32 RandomIndex = FMath::RandRange(0, AvailableObjectives.Num() - 1);
	if (AObjectiveBase* SelectedObjective = AvailableObjectives[RandomIndex])
	{
		SelectedObjective->ResetObjective();
		SelectedObjective->SetIsActive(true);
		SelectedObjective->StartMalfunctionTimer(MalfunctionTimer, MalfunctionInterval, MalfunctionDamage);
		CreateObjectiveUIListItem(SelectedObjective->GetObjectiveName(), SelectedObjective);
		
		//UE_LOG(LogTemp, Warning, TEXT("Objective activated: %s"), *SelectedObjective->GetName());
	}
}

void UObjectiveManagerSubsystem::RegisterCompletedObjective(AObjectiveBase* CompletedObjective)
{
	LastCompletedObjective = CompletedObjective;
	CompletedObjective->StopMalfunctioning();
	CompletedObjective->SetIsActive(false);
	CompletedObjectives++;
	//UE_LOG(LogTemp, Warning, TEXT("Completed Objectives: %d"), CompletedObjectives);
}

void UObjectiveManagerSubsystem::ResetAllObjectives()
{
	for (AObjectiveBase* Objective : ObjectivesInLevel)
	{
		Objective->ResetObjective();
	}
}

TArray<AObjectiveBase*> UObjectiveManagerSubsystem::GetAllObjectives() const
{
	return ObjectivesInLevel;
}

void UObjectiveManagerSubsystem::CreateObjectiveUIListItem(FString ObjectiveName, AObjectiveBase* Objective)
{
	if (CreateObjectiveListItemDelegate.IsBound())
	{
		CreateObjectiveListItemDelegate.Broadcast(ObjectiveName, Objective);
	}
}

void UObjectiveManagerSubsystem::FindObjectivesInLevel()
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