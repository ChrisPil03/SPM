// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnManager.generated.h"

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

	TArray<AEnemyAI*> GetAliveEnemies();
	
	TArray<AEnemyAI*> GetDeadEnemies();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle PeriodicSpawnTimerHandle;
	FTimerHandle SpawnRateIncreaseHandle;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	void SetSpawnTimer();

	void SpawnEnemies();
	
	TSubclassOf<AEnemySpawner> EnemySpawnerClass;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	TArray<UObject*> EnemySpawners;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double SpawnRate;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double SpawnRateIncreaseTimer;

	TArray<AEnemyAI*> AliveEnemies;

	TArray<AEnemyAI*> DeadEnemies;

};
