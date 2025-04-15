// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectivePressButtonsManager.h"

#include "ObjectiveButton.h"

// Sets default values
AObjectivePressButtonsManager::AObjectivePressButtonsManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AObjectivePressButtonsManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AObjectivePressButtonsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AObjectivePressButtonsManager::RegisterButtonPressed()
{
	if(!ObjectiveInProgress)
	{
		StartObjective();
	}

	ButtonsPressed++;

	if(ButtonsPressed == AllButtons.Num())
	{
		CompleteObjective();
	}
}

void AObjectivePressButtonsManager::ResetObjective()
{
	for (AObjectiveButton* Button : AllButtons)
	{
		Button->ResetButton();
	}
	ButtonsPressed = 0;
	ObjectiveInProgress = false;
	GetWorldTimerManager().ClearTimer(ObjectiveTimer);
}

void AObjectivePressButtonsManager::StartObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective started"));
	
	GetWorldTimerManager().SetTimer(ObjectiveTimer, this, &AObjectivePressButtonsManager::OnTimerEnd, TimeLimit, false);
}

void AObjectivePressButtonsManager::ProgressObjective()
{
	ObjectiveProgress = GetWorldTimerManager().GetTimerElapsed(ObjectiveTimer) / TimeLimit;
}

void AObjectivePressButtonsManager::CompleteObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective complete"));

	ObjectiveProgress = 1.0f;
	ObjectiveComplete = true;
	ObjectiveInProgress = false;
}

void AObjectivePressButtonsManager::OnTimerEnd()
{
	if(!ObjectiveComplete)
	{
		ResetObjective();
	}
}

