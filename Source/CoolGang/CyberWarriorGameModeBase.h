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

	virtual bool GameIsOver() const;
	
protected:
	virtual void BeginPlay() override;
	bool bGameHasEnded;
	
};
