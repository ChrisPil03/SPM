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
	ADiveGameMode();
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerKilled(APlayerCharacter* PlayerKilled) override;

	float GetRemainingGameTime() const;

protected:
	
	 virtual void BeginPlay() override;
	
private:
	void EndGame() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float TimerTime = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	int32 EnemiesKilled = 0;

};
