// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CyberWarriorGameModeBase.h"
#include "DiveGameMode.generated.h"

class ASystemIntegrity;

UCLASS()
class COOLGANG_API ADiveGameMode : public ACyberWarriorGameModeBase
{
	GENERATED_BODY()

public:
	ADiveGameMode();
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerKilled(APlayerCharacter* PlayerKilled) override;
	
	float GetTimeUntilNextObjective() const;
	
protected:
	
	 virtual void BeginPlay() override;
	
private:
	void EndGame() const;

	static float ComputeTimer(int cycleIndex, float T0 = 180.0f, float Tmin = 50.0f, float k  = 20.0f);
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float BaselineObjectiveTimer = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float MinimumObjectiveTimer = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float TimeScalingValue = 30.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float NextObjectiveTimer = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float ObjectiveMalfunctionTimer = 30.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	int32 EnemiesKilled = 0;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	int32 ObjectiveCount = 0;
};
