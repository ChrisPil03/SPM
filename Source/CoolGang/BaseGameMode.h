// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()


private:
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	int EnemiesKilled = 0;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	float GameTimer = 0;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	bool bIsObjectiveCompleted = false; 
};
