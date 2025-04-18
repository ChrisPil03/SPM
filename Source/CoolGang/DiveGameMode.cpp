// Fill out your copyright notice in the Description page of Project Settings.


#include "DiveGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"


void ADiveGameMode::PlayerKilled(APlayerCharacter* PlayerKilled)
{
	Super::PlayerKilled(PlayerKilled);
	APlayerController* PlayerController = Cast<APlayerController>(PlayerKilled->GetController());
	if (PlayerController != nullptr)
	{
		EndGame();
	}
}

void ADiveGameMode::EndGame() const
{
	AController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->GameHasEnded(Controller->GetPawn(), false);
}

