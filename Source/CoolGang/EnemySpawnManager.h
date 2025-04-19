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

	const TArray<AEnemyAI*>& GetAliveEnemies() const;
	
	const  TArray<AEnemyAI*>& GetDeadEnemies() const;
	
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

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	TSubclassOf<AEnemySpawner> EnemySpawnerClass;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	TArray<AEnemySpawner*> EnemySpawners;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double SpawnRate;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	double SpawnRateIncreaseTimer;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	int MaximumEnemies;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	TArray<AEnemyAI*> AliveEnemies;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	TArray<AEnemyAI*> DeadEnemies;

};
