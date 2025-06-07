// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CyberWarriorGameModeBase.h"
#include "DiveGameMode.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnMinutePassed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);

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

	void EndGame();

	UFUNCTION(BlueprintPure, Category = "Timer")
	void GetElapsedMinutesAndSeconds(int32& OutMinutes, int32& OutSeconds) const;

	template <typename T>
	void AddOnMinutePassedFunction(T* Object, void (T::*Func)())
	{
		OnMinutePassed.AddUObject(Object, Func);
	}

protected:
	
	 virtual void BeginPlay() override;
	
private:
	static float CalculateObjectiveTimer(int cycleIndex, float T0 = 180.0f, float Tmin = 50.0f, float k  = 20.0f);
	void HandleElapsedTime(const float DeltaTime);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective", meta = (AllowPrivateAccess))
	float BaselineObjectiveTimer = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective", meta = (AllowPrivateAccess))
	float MinimumObjectiveTimer = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective", meta = (AllowPrivateAccess))
	float TimeScalingValue = 2.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective", meta = (AllowPrivateAccess))
	float NextObjectiveTimer = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective", meta = (AllowPrivateAccess))
	float ObjectiveMalfunctionTimer = 30.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	int32 EnemiesKilled = 0;

	UPROPERTY(VisibleAnywhere, Category="Objective", meta = (AllowPrivateAccess))
	int32 ObjectiveCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Timer")
	float ElapsedTime = 0.f;

	UPROPERTY(BlueprintAssignable, meta = (AllowPrivateAccess))
	FOnGameOver OnGameOver;
	
	FOnMinutePassed OnMinutePassed;


	static float CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent);
	
	UPROPERTY(EditDefaultsOnly, Category="Spawning", meta = (AllowPrivateAccess))
	double BaselineSpawnInterval = 4.0;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning", meta = (AllowPrivateAccess))
	double MinimumSpawnInterval = 0.2;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning", meta = (AllowPrivateAccess))
	int32 MaxSpawnIntervalIncreaseCount = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning", meta = (AllowPrivateAccess))
	double SpawnIntervalScale = 2.5;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning", meta = (AllowPrivateAccess))
	double SpawnAccelerationRate = 3.2;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning", meta = (AllowPrivateAccess))
	double SpawnIntervalIncreaseTimer = 20.0;

	double UpdatedSpawnInterval;
	double SpawnInterval;
	int32 SpawnIntervalIncreaseCount;
	double SpawnIntervalIncreaseProgress;

	
};
