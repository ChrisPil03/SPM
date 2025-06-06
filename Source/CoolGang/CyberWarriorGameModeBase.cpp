// Fill out your copyright notice in the Description page of Project Settings.


#include "CyberWarriorGameModeBase.h"

void ACyberWarriorGameModeBase::PlayerKilled(APlayerCharacter* PlayerKilled)
{
}

bool ACyberWarriorGameModeBase::GameIsOver() const
{
	return bGameHasEnded;
}

bool ACyberWarriorGameModeBase::IsGameActive() const
{
	return bGameIsActive;
}

void ACyberWarriorGameModeBase::SetGameActiveState(bool State)
{
	bGameIsActive = State;
}

void ACyberWarriorGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	bGameIsActive = false;
	bGameHasEnded = false;
}
