// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawnManager.h"
#include "EnemySpawner.h"
#include "EnemyAI.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemySpawnManager::AEnemySpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AEnemySpawnManager::MarkEnemyAsDead(AEnemyAI* Enemy)
{
	UE_LOG(LogTemp, Warning, TEXT("Amount of enemies: %d"), AliveEnemies.Num());
	UE_LOG(LogTemp, Warning, TEXT("Alive enemies contains %s: %d"), *Enemy->GetActorNameOrLabel(), AliveEnemies.Contains(Enemy));
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
	}
	AliveEnemies.Add(Enemy);
}

const TArray<AEnemyAI*>& AEnemySpawnManager::GetAliveEnemies() const 
{
	return AliveEnemies;
}

const TArray<AEnemyAI*>& AEnemySpawnManager::GetDeadEnemies() const 
{
	return DeadEnemies;
}

// Called when the game starts or when spawned
void AEnemySpawnManager::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(
		PeriodicSpawnTimerHandle,
		this,
		&AEnemySpawnManager::SetSpawnTimer,
		SpawnRateIncreaseTimer,
		true,
		0
		);

	TArray<UObject*> FoundObjects;
	if (EnemySpawnerClass)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), EnemySpawnerClass, FoundActors);
		
		EnemySpawners.Empty();
		EnemySpawners.Reserve(FoundActors.Num());
		
		if (FoundActors.Num() > 0)
		{
			for (AActor* FoundActor : FoundActors)
			{
				AEnemySpawner* SpecificSpawner = Cast<AEnemySpawner>(FoundActor);
        		
				if (SpecificSpawner)
				{
					EnemySpawners.Add(SpecificSpawner);
				}
			}
		}
	}
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
		
		if (AliveEnemies.Num() >= MaximumEnemies)
		{
			return;
		}
		
		AEnemySpawner* Spawner = Cast<AEnemySpawner>(EnemySpawner);
		if (Spawner)
		{
			UE_LOG(LogTemp, Warning, TEXT("Marking enemy as alive"));
			AEnemyAI* Enemy = Spawner->SpawnEnemy();
			MarkEnemyAsAlive(Enemy);
		}
	}
}

