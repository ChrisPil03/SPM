// Fill out your copyright notice in the Description page of Project Settings.


#include "DiveGameMode.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"



void ADiveGameMode::BeginPlay()
{
	Super::BeginPlay();
	
}


void ADiveGameMode::PlayerKilled(APlayerCharacter* PlayerKilled)
{
	Super::PlayerKilled(PlayerKilled);
	APlayerController* PlayerController = Cast<APlayerController>(PlayerKilled->GetController());
	if (PlayerController != nullptr)
	{
		EndGame();
	}
}

float ADiveGameMode::GetRemainingGameTime() const
{
	return TimerTime;
}

ADiveGameMode::ADiveGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ADiveGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TimerTime -= DeltaSeconds;
	if (TimerTime <= 0)
	{
		EndGame();
	}
}

void ADiveGameMode::EndGame() const
{
	AController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->GameHasEnded(Controller->GetPawn(), false);
}
