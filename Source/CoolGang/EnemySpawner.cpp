// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "TimerManager.h"
#include "EnemySpawnManagerSubsystem.h"
#include "EnemyAI.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	EnemySpawnManager = GetWorld()->GetSubsystem<UEnemySpawnManagerSubsystem>();

    if (!SpawnArea) return;
	EnemySpawnManager->RegisterSpawner(SpawnArea, this);
    // UE_LOG(LogTemp, Warning, TEXT("EnemySpawner Registering"))
}


void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AEnemyAI* AEnemySpawner::ReuseDeadEnemy(AEnemyAI* Enemy) const
{
	if (Enemy)
	{
		SetEnemyPosition(Enemy);
		Enemy->SetAlive();
	}
	return Enemy;
}

void AEnemySpawner::SetEnemyPosition(AEnemyAI* Enemy) const
{
	if (!IsValid(Enemy))
	{
		return;
	}
	FVector Location = GetActorLocation();
	FRotator Rotation = GetActorRotation();
	Enemy->SetActorLocationAndRotation(Location, Rotation);
}

AEnemyAI* AEnemySpawner::SpawnEnemy(const TSubclassOf<AEnemyAI>& EnemyClass) const
{
    if (!EnemyClass)
    {
        return nullptr;
    }

    if (!EnemySpawnManager)
    {
        return nullptr;
    }

    FVector Location = GetActorLocation();
    FRotator Rotation = GetActorRotation();
    
    auto AliveMap = EnemySpawnManager->GetAliveEnemiesMap();
    auto DeadMap = EnemySpawnManager->GetDeadEnemiesMap();

    TArray<AEnemyAI*> AliveEnemies;
    TArray<AEnemyAI*> DeadEnemies;
    
    if (AliveMap.Num() > 0)
    {
        const FEnemyArrayWrapper* AliveEnemyArrayWrapper = AliveMap.Find(EnemyClass);
        if (AliveEnemyArrayWrapper)
        {
            AliveEnemies = AliveEnemyArrayWrapper->Enemies;
        }
    }
    
    if (DeadMap.Num() > 0)
    {
        const FEnemyArrayWrapper* DeadEnemyArrayWrapper = DeadMap.Find(EnemyClass);
        if (DeadEnemyArrayWrapper)
        {
            DeadEnemies = DeadEnemyArrayWrapper->Enemies;
        }
    }

    int MaxForType = EnemySpawnManager->GetMaxEnemiesByType(EnemyClass);

    if (AliveEnemies.Num() < MaxForType)
    {
        if (DeadEnemies.Num() > 0)
        {
            AEnemyAI* ReusedEnemy = ReuseDeadEnemy(DeadEnemies[0]);
            return ReusedEnemy;
        }
        AEnemyAI* NewEnemy = GetWorld()->SpawnActor<AEnemyAI>(EnemyClass, Location, Rotation);
        if (NewEnemy)
        {
        	
            EnemySpawnManager->MarkEnemyAsAlive(NewEnemy);
        }
        return NewEnemy;
        
    }
	return nullptr;
}

void AEnemySpawner::RelocateEnemy(AEnemyAI* Enemy) const
{
	SetEnemyPosition(Enemy);
}