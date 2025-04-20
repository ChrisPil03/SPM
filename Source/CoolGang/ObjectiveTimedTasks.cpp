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
}

void AObjectiveTimedTasks::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsComplete())
	{
		return;
	}

	if (GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaTime);
	}
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

	if (GetObjectiveProgress() == FProgressTimer::FullCompletion)
	{
		ResetObjective();
	}
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
