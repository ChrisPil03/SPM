#include "ObjectiveTimedTasks.h"
#include "InteractableObject.h"

AObjectiveTimedTasks::AObjectiveTimedTasks()
{
	InteractedTasks = 0;
	SetIsTimeBased(true);
	ScoreType = EScoreType::ObjectiveButtonsCompleted;
}

void AObjectiveTimedTasks::BeginPlay()
{
	Super::BeginPlay();

	for (AInteractableObject* Interactable : AllInteractableObjects)
	{
		Interactable->InteractDelegate.AddDynamic(this, &AObjectiveTimedTasks::RegisterInteraction);
	}
	SetInteractablesInteractable(false);
	FTimerCompletionDelegate Delegate;
	Delegate.BindUObject(this, &AObjectiveBase::FailObjective);
	GetProgressTimer().SetCompletionDelegate(Delegate);
}

void AObjectiveTimedTasks::ResetObjective()
{
	Super::ResetObjective();

	for (AInteractableObject* Interactable : AllInteractableObjects)
	{
		Interactable->ResetInteractable();
	}
	InteractedTasks = 0;

	if (OnUniqueProgressChanged.IsBound())
	{
		OnUniqueProgressChanged.Broadcast();
	}
}

void AObjectiveTimedTasks::CompleteObjective()
{
	OnRequestAddScore.Broadcast(ScoreType);
	Super::CompleteObjective();
}

void AObjectiveTimedTasks::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaSeconds);

		if (OnUniqueProgressChanged.IsBound())
		{
			OnUniqueProgressChanged.Broadcast();
		}
	}
}

void AObjectiveTimedTasks::SetIsActive(const bool bNewState)
{
	Super::SetIsActive(bNewState);
	SetInteractablesInteractable(bNewState);
}

float AObjectiveTimedTasks::GetObjectiveProgress() const
{
	if (AllInteractableObjects.Num() > 0)
	{
		return static_cast<float>(InteractedTasks) / AllInteractableObjects.Num();
	}
	return 0;
}

TArray<FString> AObjectiveTimedTasks::GetUniqueObjectiveProgress() const
{
	int32 OutMinutes;
	int32 OutSeconds;
	GetTimeUntilFailure(OutMinutes, OutSeconds);
	FString Minutes;
	FString Seconds;
	if (OutMinutes <= 9)
	{
		Minutes = FString::Printf(TEXT("0%d"), OutMinutes);
	}else
	{
		Minutes = FString::Printf(TEXT("%d"), OutMinutes);
	}
	if (OutSeconds <= 9)
	{
		Seconds = FString::Printf(TEXT("0%d"), OutSeconds);
	}else
	{
		Seconds = FString::Printf(TEXT("%d"), OutSeconds);
	}
	
	return {
		FString::Printf(TEXT("Circuit breaks pulled: %d / %d"), GetInteractions(), GetTotalInteractables()),
		FString::Printf(TEXT("Time left: %s:%s"), *Minutes, *Seconds)
	};
}

void AObjectiveTimedTasks::GetTimeUntilFailure(int32& OutMinutes, int32& OutSeconds) const
{
	int32 ElapsedTime = GetProgressTimer().GetElapsedTime();
	int32 TimeLeft = GetObjectiveTime() - ElapsedTime;
	OutMinutes = TimeLeft / 60;
	OutSeconds = TimeLeft % 60;
}

void AObjectiveTimedTasks::RegisterInteraction(AInteractableObject* InteractableObject)
{
	if (GetIsNotStarted())
	{
		StartObjective();
	}

	InteractedTasks++;

	if (OnUniqueProgressChanged.IsBound())
	{
		OnUniqueProgressChanged.Broadcast();
	}

	if (InteractedTasks == AllInteractableObjects.Num())
	{
		CompleteObjective();
	}
}

void AObjectiveTimedTasks::SetInteractablesInteractable(const bool bNewState)
{
	for (AInteractableObject* Object : AllInteractableObjects)
	{
		Object->SetCanInteractWith(bNewState);
	}
}
