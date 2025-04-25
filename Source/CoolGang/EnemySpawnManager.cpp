// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawnManager.h"
#include "EnemySpawner.h"
#include "EnemyAI.h"
#include "PlayerLocationDetection.h"
#include "Kismet/GameplayStatics.h"

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

	BindPlayerLocationDetection();
}

// Called every frame
void AEnemySpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SpawnInterval <= 0.f)
	{
		SpawnEnemy();
		SpawnInterval = BaselineSpawnInterval;
	}
	SpawnInterval -= DeltaTime;
	
	if (SpawnIntervalIncreaseProgress <= 0.f)
	{
		BaselineSpawnInterval = CalculateSpawnTimer(SpawnIntervalIncreaseCount++, BaselineSpawnInterval, MinimumSpawnInterval, SpawnIntervalScale);
		SpawnIntervalIncreaseProgress = SpawnIntervalIncreaseTimer;
	}
	SpawnIntervalIncreaseProgress -= DeltaTime;
	
	
}

void AEnemySpawnManager::RegisterSpawner(APlayerLocationDetection* SpawnLocation, AEnemySpawner* Spawner)
{
	SpawnersByLocation.FindOrAdd(SpawnLocation).Add(Spawner);
}


float AEnemySpawnManager::CalculateSpawnTimer(int cycleIndex, float T0, float Tmin, float k)
{
	float t = T0 - k * std::logf(cycleIndex + 1);
	return std::max(Tmin, t);
}

void AEnemySpawnManager::SpawnEnemy()
{
	if (AliveEnemies.Num() >= MaximumEnemies)
	{
		return;
	}
	
	if (CopyCurrentEnemySpawners.Num() == 0)
	{
		CopyCurrentEnemySpawners = CurrentEnemySpawners;
	}
	int32 RandomIndex = FMath::RandRange(0, CopyCurrentEnemySpawners.Num() - 1);

	if (CopyCurrentEnemySpawners.Num() == 0)
	{
		return;
	}
	AEnemyAI* Enemy = CopyCurrentEnemySpawners[RandomIndex]->SpawnEnemy();
	MarkEnemyAsAlive(Enemy);
	CopyCurrentEnemySpawners.RemoveAt(RandomIndex);
}

void AEnemySpawnManager::BindPlayerLocationDetection()
{
	TArray<AActor*> FoundLocations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerLocationDetection::StaticClass(), FoundLocations);

	for (AActor* FoundActor : FoundLocations)
	{
		if (APlayerLocationDetection* PlayerLocationDetection = Cast<APlayerLocationDetection>(FoundActor))
		{
		
			FOnTriggerEnterDelegate EnterDelegate;
			EnterDelegate.AddUObject(this, &AEnemySpawnManager::OnEnterTriggerBox);
			PlayerLocationDetection->SetOnTriggerEnter(EnterDelegate);

			FOnTriggerExitDelegate ExitDelegate;
			ExitDelegate.AddUObject(this, &AEnemySpawnManager::OnExitTriggerBox);
			PlayerLocationDetection->SetOnTriggerExit(ExitDelegate);
		}
	}
	
}

void AEnemySpawnManager::OnEnterTriggerBox(APlayerLocationDetection* SpawnBox)
{
	UE_LOG(LogTemp, Display, TEXT("Entered enemy trigger box"));
	for (AEnemySpawner* Spawner : *SpawnersByLocation.Find(SpawnBox))
	{
		CurrentEnemySpawners.Add(Spawner);
	}
	CopyCurrentEnemySpawners.Empty();
}

void AEnemySpawnManager::OnExitTriggerBox(APlayerLocationDetection* SpawnBox)
{
	UE_LOG(LogTemp, Display, TEXT("Exited enemy trigger box"));
	for (AEnemySpawner* Spawner : *SpawnersByLocation.Find(SpawnBox))
	{
		CurrentEnemySpawners.Remove(Spawner);
	}
	CopyCurrentEnemySpawners.Empty();
}
