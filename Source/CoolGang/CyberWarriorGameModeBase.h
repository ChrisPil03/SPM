// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "CyberWarriorGameModeBase.generated.h"

class APlayerCharacter;
/**
 * 
 */
UCLASS()
class COOLGANG_API ACyberWarriorGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PlayerKilled(APlayerCharacter* PlayerKilled);

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	int EnemiesKilled = 0;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	float GameTimer = 0;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	bool bIsObjectiveCompleted = false; 
};
