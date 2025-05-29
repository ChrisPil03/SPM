#include "ObjectiveManagerSubsystem.h"
#include "ObjectiveBase.h"
#include "ObjectiveDefendGenerator.h"
#include "Kismet/GameplayStatics.h"

UObjectiveManagerSubsystem::UObjectiveManagerSubsystem() :
	CompletedSubObjectives(0),
	CompletionsMainObjective(0)
{
	SubObjectivesInLevel.Empty();
	MainObjective = nullptr;
	LastCompletedObjective = nullptr;
}

void UObjectiveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UObjectiveManagerSubsystem::OnWorldInitialized);
	
}

void UObjectiveManagerSubsystem::OnWorldInitialized(const UWorld::FActorsInitializedParams& Params)
{
	FindObjectivesInLevel();
}

void UObjectiveManagerSubsystem::DeactivateAllSubObjectives()
{
	for (AObjectiveBase* SubObjective : SubObjectivesInLevel)
	{
		if (SubObjective->GetIsActive() && SubObjective->GetIsNotStarted())
		{
			SubObjective->ResetObjective();
			SubObjective->SetIsActive(false);
		}
	}
}

void UObjectiveManagerSubsystem::ActivateRandomObjective(float MalfunctionTimer, float MalfunctionInterval, float MalfunctionDamage)
{
	if (SubObjectivesInLevel.IsEmpty() ||
		(MainObjective && (MainObjective->GetIsActive() || MainObjective->GetIsActivating())))
	{
		return;
	}
	TArray<AObjectiveBase*> AvailableObjectives;
	for (AObjectiveBase* Objective : SubObjectivesInLevel)
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
		ShowObjectiveUIListItem(SelectedObjective->GetObjectiveName(), SelectedObjective);
		SelectedObjective->SetIsActive(true);
		//SelectedObjective->StartMalfunctionTimer(MalfunctionTimer, MalfunctionInterval, MalfunctionDamage);
		BroadcastActivatedObjective(SelectedObjective);
		//UE_LOG(LogTemp, Warning, TEXT("Objective activated: %s"), *SelectedObjective->GetName());
	}
}

void UObjectiveManagerSubsystem::ActivateMainObjective()
{
	if (MainObjective)
	{
		// DeactivateAllSubObjectives();
		ShowObjectiveUIListItem(MainObjective->GetObjectiveName(), MainObjective);
		MainObjective->SetIsActive(true);
		BroadcastActivatedObjective(MainObjective);
	}
}

void UObjectiveManagerSubsystem::RegisterCompletedObjective(AObjectiveBase* CompletedObjective)
{
	LastCompletedObjective = CompletedObjective;
	// CompletedObjective->StopMalfunctioning();
	CompletedObjective->SetIsActive(false);
	CompletedSubObjectives++;
	BroadcastCompletedObjective(CompletedObjective);
	//UE_LOG(LogTemp, Warning, TEXT("Completed Objectives: %d"), CompletedObjectives);
}

void UObjectiveManagerSubsystem::RegisterFailedObjective(AObjectiveBase* FailedObjective)
{
	LastCompletedObjective = FailedObjective;
	BroadcastFailedObjective(FailedObjective);
}

void UObjectiveManagerSubsystem::ResetAllObjectives()
{
	for (AObjectiveBase* Objective : SubObjectivesInLevel)
	{
		Objective->ResetObjective();
	}
}

TArray<AObjectiveBase*> UObjectiveManagerSubsystem::GetAllSubObjectives() const
{
	return SubObjectivesInLevel;
}

AObjectiveDefendGenerator* UObjectiveManagerSubsystem::GetMainObjective() const
{
	return MainObjective;
}

void UObjectiveManagerSubsystem::ShowObjectiveUIListItem(FString ObjectiveName, AObjectiveBase* Objective)
{
	if (ShowObjectiveListItem.IsBound())
	{
		ShowObjectiveListItem.Broadcast(ObjectiveName, Objective);
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
			if (AObjectiveDefendGenerator* Main = Cast<AObjectiveDefendGenerator>(Objective))
			{
				MainObjective = Main;
			}else
			{
				SubObjectivesInLevel.Add(Objective);	
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("MainObjective: %hd"), MainObjective != nullptr);
}

void UObjectiveManagerSubsystem::BroadcastCompletedObjective(AObjectiveBase* Objective) const
{
	if (OnCompletedObjective.IsBound())
	{
		OnCompletedObjective.Broadcast(Objective);
	}
}

void UObjectiveManagerSubsystem::BroadcastFailedObjective(AObjectiveBase* Objective) const
{
	if (OnFailedObjective.IsBound())
	{
		OnFailedObjective.Broadcast(Objective);
	}
}

void UObjectiveManagerSubsystem::BroadcastActivatedObjective(AObjectiveBase* Objective) const
{
	if (OnActivatedObjective.IsBound())
	{
		OnActivatedObjective.Broadcast(Objective);
	}
}