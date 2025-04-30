// Fill out your copyright notice in the Description page of Project Settings.


#include "CyberWarriorGameModeBase.h"

void ACyberWarriorGameModeBase::PlayerKilled(APlayerCharacter* PlayerKilled)
{
}

bool ACyberWarriorGameModeBase::GameIsOver() const
{
	return bGameHasEnded;
}

void ACyberWarriorGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	bGameHasEnded = false;
}
