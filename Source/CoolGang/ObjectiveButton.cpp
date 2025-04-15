// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveButton.h"

#include "ObjectivePressButtonsManager.h"

// Sets default values
AObjectiveButton::AObjectiveButton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AObjectiveButton::BeginPlay()
{
	Super::BeginPlay();
	
}

void AObjectiveButton::Interact(AActor* Interactor)
{
	if (bPressed || !ObjectiveManager)
	{
		return;
	}

	PressButton();
}

void AObjectiveButton::PressButton()
{
	bPressed = true;
	ObjectiveManager->RegisterButtonPressed();
}

void AObjectiveButton::ResetButton()
{
	bPressed = false;
}


