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

	SpawnInterval = BaselineSpawnInterval;
	
	BindPlayerLocationDetection();
}

// Called every frame
void AEnemySpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SpawnInterval <= 0.f)
	{
		SpawnEnemy();
		SpawnInterval = UpdatedSpawnInterval;
	}
	SpawnInterval -= DeltaTime;
	
	if (SpawnIntervalIncreaseProgress <= 0.f)
	{
		double OldSpawnInterval = UpdatedSpawnInterval;
		UpdatedSpawnInterval = CalculateSpawnTimer(SpawnIntervalIncreaseCount++, BaselineSpawnInterval, MinimumSpawnInterval, SpawnIntervalScale, MaxSpawnIntervalIncreaseCount, SpawnAccelerationRate);
		//UE_LOG(LogTemp, Warning, TEXT("Diff: %f"), OldSpawnInterval-UpdatedSpawnInterval);
		SpawnIntervalIncreaseProgress = SpawnIntervalIncreaseTimer;
	}
	SpawnIntervalIncreaseProgress -= DeltaTime;
	
	
}

void AEnemySpawnManager::RegisterSpawner(APlayerLocationDetection* SpawnLocation, AEnemySpawner* Spawner)
{
	SpawnersByLocation.FindOrAdd(SpawnLocation).Add(Spawner);
}


float AEnemySpawnManager::CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent)
{
	// Normalize cycle progress between 0..1
	float tNorm = float(cycleIndex) / float(maxCycles);

	// Compute the “ease-in” curve via power
	float deltaNorm = pow(tNorm, exponent);

	// Scale and invert to get your timer
	float t = baselineInterval
			- intervalScale * deltaNorm;

	// Clamp to your minimum
	return std::max(minimumInterval, t);
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
			PlayerLocationDetection->AddOnTriggerEnterFunction(this, &AEnemySpawnManager::OnEnterTriggerBox);
			PlayerLocationDetection->AddOnTriggerExitFunction(this, &AEnemySpawnManager::OnExitTriggerBox);
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
