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
    // --- Initial Validations & Logging ---
    if (!EnemyClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("AEnemySpawner::SpawnEnemy: PRE-CHECK - EnemyClass is null. Returning nullptr."));
        return nullptr;
    }

    if (!EnemySpawnManager)
    {
        UE_LOG(LogTemp, Error, TEXT("AEnemySpawner::SpawnEnemy: PRE-CHECK - EnemySpawnManager is null for EnemyClass '%s'! Returning nullptr."), *EnemyClass->GetName());
        return nullptr;
    }

    // UE_LOG(LogTemp, Log, TEXT("AEnemySpawner::SpawnEnemy: ATTEMPTING to spawn/reuse EnemyClass '%s'."), *EnemyClass->GetName());

    // --- Get Actor Transform ---
    FVector Location = GetActorLocation();
    FRotator Rotation = GetActorRotation();

    // --- Retrieve Enemy Maps ---
    // Assuming these getters return copies or const references as appropriate
    auto AliveMap = EnemySpawnManager->GetAliveEnemiesMap();
    auto DeadMap = EnemySpawnManager->GetDeadEnemiesMap();
    // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: Retrieved AliveMap (Size: %d) and DeadMap (Size: %d)."), AliveMap.Num(), DeadMap.Num());

    // --- Prepare Local TArrays (as per your original structure) ---
    TArray<AEnemyAI*> AliveEnemies; // Will store enemies of the specific EnemyClass
    TArray<AEnemyAI*> DeadEnemies;   // Will store enemies of the specific EnemyClass

    // --- Populate AliveEnemies for the specific EnemyClass ---
    if (AliveMap.Num() > 0) // Check if the overall map has any entries
    {
        const FEnemyArrayWrapper* AliveEnemyArrayWrapper = AliveMap.Find(EnemyClass); // Use Find to avoid adding keys
        if (AliveEnemyArrayWrapper)
        {
            AliveEnemies = AliveEnemyArrayWrapper->Enemies; // Copy the array
            // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: Found ALIVE list for '%s'. Count: %d."), *EnemyClass->GetName(), AliveEnemies.Num());
        }
        else
        {
            // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: No ALIVE list found in AliveMap for '%s'. Assuming 0 alive."), *EnemyClass->GetName());
            // AliveEnemies will remain empty, which is correct if no entry for EnemyClass
        }
    }
    else
    {
        // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: Overall AliveMap is empty."));
    }

    // --- Populate DeadEnemies for the specific EnemyClass ---
    // *** CRITICAL FIX from your original code: You were finding in AliveMap again instead of DeadMap ***
    if (DeadMap.Num() > 0) // Check if the overall map has any entries
    {
        // const FEnemyArrayWrapper* DeadEnemyArrayWrapper = AliveMap.Find(EnemyClass); // <<< ORIGINAL BUG WAS HERE
        const FEnemyArrayWrapper* DeadEnemyArrayWrapper = DeadMap.Find(EnemyClass); // <<< CORRECTED TO DeadMap
        if (DeadEnemyArrayWrapper)
        {
            DeadEnemies = DeadEnemyArrayWrapper->Enemies; // Copy the array
            // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: Found DEAD list for '%s'. Count: %d."), *EnemyClass->GetName(), DeadEnemies.Num());
        }
        else
        {
            // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: No DEAD list found in DeadMap for '%s'. Assuming 0 dead."), *EnemyClass->GetName());
            // DeadEnemies will remain empty
        }
    }
    else
    {
        // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: Overall DeadMap is empty."));
    }
    
    // --- Calculate Total and Check Cap ---
    int TotalSpawnedEnemiesOfType = AliveEnemies.Num() + DeadEnemies.Num();
    // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: Total EXISTING instances for '%s': %d (Alive: %d, Dead: %d)"),
    //     *EnemyClass->GetName(), TotalSpawnedEnemiesOfType, AliveEnemies.Num(), DeadEnemies.Num());

    int MaxForType = EnemySpawnManager->GetMaxEnemiesByType(EnemyClass); // Assuming this function exists and is correct
    // UE_LOG(LogTemp, Verbose, TEXT("AEnemySpawner::SpawnEnemy: MAX instances configured for '%s': %d"), *EnemyClass->GetName(), MaxForType);

    if (AliveEnemies.Num() < MaxForType)
    {
        // UE_LOG(LogTemp, Log, TEXT("AEnemySpawner::SpawnEnemy: Under cap for '%s' (Total: %d < Max: %d). Proceeding with spawn/reuse."),
        //     *EnemyClass->GetName(), TotalSpawnedEnemiesOfType, MaxForType);

        if (DeadEnemies.Num() > 0)
        {
            // UE_LOG(LogTemp, Log, TEXT("AEnemySpawner::SpawnEnemy: Dead enemies available for '%s' (Count: %d). Attempting to REUSE DeadEnemies[0]."),
            //     *EnemyClass->GetName(), DeadEnemies.Num());
            // The ReuseDeadEnemy function is critical here. It needs to:
            // 1. Remove DeadEnemies[0] from the DeadMap in the EnemySpawnManager.
            // 2. Reset DeadEnemies[0]'s state.
            // 3. Add DeadEnemies[0] to the AliveMap in the EnemySpawnManager (e.g., via MarkEnemyAsAlive).
            AEnemyAI* ReusedEnemy = ReuseDeadEnemy(DeadEnemies[0]); // Assuming this exists and does the above
            if (ReusedEnemy)
            {
                // UE_LOG(LogTemp, Log, TEXT("AEnemySpawner::SpawnEnemy: SUCCESSFULLY REUSED enemy '%s' for class '%s'."), *ReusedEnemy->GetName(), *EnemyClass->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AEnemySpawner::SpawnEnemy: ReuseDeadEnemy FAILED for class '%s' even though DeadEnemies.Num() > 0."), *EnemyClass->GetName());
            }
            return ReusedEnemy;
        }
        else // No dead enemies to reuse, spawn new
        {
            // UE_LOG(LogTemp, Log, TEXT("AEnemySpawner::SpawnEnemy: No dead enemies for '%s'. Attempting to SPAWN NEW instance."), *EnemyClass->GetName());
            AEnemyAI* NewEnemy = GetWorld()->SpawnActor<AEnemyAI>(EnemyClass, Location, Rotation);
            if (NewEnemy)
            {
                // UE_LOG(LogTemp, Log, TEXT("AEnemySpawner::SpawnEnemy: SUCCESSFULLY SPAWNED new enemy '%s' of type '%s'."), *NewEnemy->GetName(), *EnemyClass->GetName());
                // IMPORTANT: After spawning, you MUST inform EnemySpawnManager:
                EnemySpawnManager->MarkEnemyAsAlive(NewEnemy);
                // EnemySpawnManager->RegisterEnemy(NewEnemy); // If needed for a different counter
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AEnemySpawner::SpawnEnemy: GetWorld()->SpawnActor FAILED for EnemyClass '%s'."), *EnemyClass->GetName());
            }
            return NewEnemy;
        }
    }
    else // Cap reached
    {
        // UE_LOG(LogTemp, Warning, TEXT("AEnemySpawner::SpawnEnemy: Max instance CAP REACHED for '%s' (Total: %d >= Max: %d). Returning nullptr."),
        //     *EnemyClass->GetName(), TotalSpawnedEnemiesOfType, MaxForType);
        return nullptr;
    }
}

void AEnemySpawner::RelocateEnemy(AEnemyAI* Enemy) const
{
	SetEnemyPosition(Enemy);
}