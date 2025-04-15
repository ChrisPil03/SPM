// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemySpawnManager.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyAI;
class AEnemySpawnManager;

UCLASS()
class COOLGANG_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

	AEnemyAI* SpawnEnemy();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning|Timing", meta = (ClampMin = "0.1"))
	float PeriodicSpawnInterval = 5.0f;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	TSubclassOf<AEnemyAI> EnemyClass;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	AEnemySpawnManager* EnemySpawnManager;
	
};


