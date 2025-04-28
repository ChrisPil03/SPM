// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnManager.generated.h"

class APlayerLocationDetection;
class AEnemySpawner;
class AEnemyAI;

UCLASS()
class COOLGANG_API AEnemySpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawnManager();

	void MarkEnemyAsAlive(AEnemyAI* Enemy);
	void MarkEnemyAsDead(AEnemyAI* Enemy);

	const TArray<AEnemyAI*>& GetAliveEnemies() const;
	
	const  TArray<AEnemyAI*>& GetDeadEnemies() const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void RegisterSpawner(APlayerLocationDetection*, AEnemySpawner*);
private:
	
	TMap<APlayerLocationDetection*, TArray<AEnemySpawner*>> SpawnersByLocation;
	
	void BindPlayerLocationDetection();
	void OnEnterTriggerBox(APlayerLocationDetection* SpawnBox);
	void OnExitTriggerBox(APlayerLocationDetection* SpawnBox);
	
	static float CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent);

	void SpawnEnemy();

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	TArray<AEnemySpawner*> CurrentEnemySpawners;

	TArray<AEnemySpawner*> CopyCurrentEnemySpawners;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double BaselineSpawnInterval;

	UPROPERTY(VisibleAnywhere)
	double UpdatedSpawnInterval;
	
	UPROPERTY(VisibleAnywhere)
	double SpawnInterval;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double MinimumSpawnInterval;

	UPROPERTY(VisibleAnywhere)
	int32 SpawnIntervalIncreaseCount;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	int32 MaxSpawnIntervalIncreaseCount;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double SpawnIntervalScale;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double SpawnAccelerationRate;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double SpawnIntervalIncreaseTimer;
	
	UPROPERTY(VisibleAnywhere)
	double SpawnIntervalIncreaseProgress;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	int MaximumEnemies;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	TArray<AEnemyAI*> AliveEnemies;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	TArray<AEnemyAI*> DeadEnemies;

};
