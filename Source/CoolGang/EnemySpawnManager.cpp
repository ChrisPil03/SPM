// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawnManager.h"
#include "EnemySpawner.h"
#include "EnemyAI.h"

// Sets default values
AEnemySpawnManager::AEnemySpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AEnemySpawnManager::MarkEnemyAsDead(AEnemyAI* Enemy)
{
	if (AliveEnemies.Contains(Enemy))
	{
		AliveEnemies.Remove(Enemy);
		DeadEnemies.Add(Enemy);
	}
}

void AEnemySpawnManager::MarkEnemyAsAlive(AEnemyAI* Enemy)
{
	if (DeadEnemies.Contains(Enemy))
	{
		DeadEnemies.Remove(Enemy);
		AliveEnemies.Add(Enemy);
	}
}

TArray<AEnemyAI*> AEnemySpawnManager::GetAliveEnemies()
{
	return AliveEnemies;
}

TArray<AEnemyAI*> AEnemySpawnManager::GetDeadEnemies()
{
	return DeadEnemies;
}

// Called when the game starts or when spawned
void AEnemySpawnManager::BeginPlay()
{
	Super::BeginPlay();
	GetObjectsOfClass(EnemySpawnerClass,EnemySpawners, true);


	GetWorldTimerManager().SetTimer(
		PeriodicSpawnTimerHandle,
		this,
		&AEnemySpawnManager::SetSpawnTimer,
		SpawnRateIncreaseTimer,
		true,
		0
		);

}

// Called every frame
void AEnemySpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AEnemySpawnManager::SetSpawnTimer()
{
	GetWorldTimerManager().SetTimer(
	PeriodicSpawnTimerHandle,
	this,
	&AEnemySpawnManager::SpawnEnemies,
	SpawnRate,
	true,
	SpawnRate
);
}

void AEnemySpawnManager::SpawnEnemies()
{
	for (auto EnemySpawner : EnemySpawners)
	{
		if (AliveEnemies.Num() >= 50)
		{
			return;
		}
		
		AEnemySpawner* Spawner = Cast<AEnemySpawner>(EnemySpawner);
		if (Spawner)
		{
			AEnemyAI* Enemy = Spawner->SpawnEnemy();
			MarkEnemyAsAlive(Enemy);
		}
	}
}