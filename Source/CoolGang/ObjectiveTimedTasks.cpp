// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveTimedTasks.h"
#include "InteractableObject.h"

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
