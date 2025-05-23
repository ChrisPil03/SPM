// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveTimedTasks.h"
#include "InteractableObject.h"
#include "ScoreManagerComponent.h"

AObjectiveTimedTasks::AObjectiveTimedTasks()
{
	InteractedTasks = 0;
	SetIsTimeBased(true);
}

void AObjectiveTimedTasks::BeginPlay()
{
	Super::BeginPlay();

	for (AInteractableObject* Interactable : AllInteractableObjects)
	{
		FPerformDelegate Delegate;
		Delegate.AddUObject(this, &AObjectiveTimedTasks::RegisterInteraction);
		Interactable->SetInteractFunction(Delegate);
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
	Super::CompleteObjective();
	OnRequestAddScore.Broadcast(EScoreType::ObjectiveButtonsCompleted);
}

void AObjectiveTimedTasks::IncreaseObjectiveProgress(float const DeltaTime)
{
	Super::IncreaseObjectiveProgress(DeltaTime);
	
	// if (GetObjectiveProgress() == FProgressTimer::FullCompletion)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("Failed"));
	// 	FailObjective();
	// }
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
		FString::Printf(TEXT("Buttons pressed: %d / %d"), GetInteractions(), GetTotalInteractables()),
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
