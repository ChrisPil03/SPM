// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CyberWarriorGameModeBase.h"
#include "DiveGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API ADiveGameMode : public ACyberWarriorGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PlayerKilled(APlayerCharacter* PlayerKilled) override;

private:
	void EndGame() const;
};
