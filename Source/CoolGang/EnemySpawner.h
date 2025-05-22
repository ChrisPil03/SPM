// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class UEnemySpawnManagerSubsystem;
class AEnemyAI;
class APlayerLocationDetection;

UCLASS()
class COOLGANG_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

	AEnemyAI* SpawnEnemy(const TSubclassOf<AEnemyAI>& EnemyClass) const;
	
	void RelocateEnemy(AEnemyAI* Enemy) const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	AEnemyAI* ReuseDeadEnemy(AEnemyAI* Enemy) const;
	
	void SetEnemyPosition(AEnemyAI* Enemy) const;
	
	UPROPERTY(EditInstanceOnly)
	APlayerLocationDetection* SpawnArea;

	UPROPERTY()
	UEnemySpawnManagerSubsystem* EnemySpawnManager;
	
};


