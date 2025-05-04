
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemySpawnManagerSubsystem.generated.h"

// Forward declarations
class APlayerLocationDetection;
class AEnemySpawner;
class AEnemyAI;

UCLASS(Blueprintable)
class COOLGANG_API UEnemySpawnManagerSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    
    void RegisterSpawner(APlayerLocationDetection* SpawnLocation, AEnemySpawner* Spawner);
    void MarkEnemyAsAlive(AEnemyAI* Enemy);
    void MarkEnemyAsDead(AEnemyAI* Enemy);
    const TArray<AEnemyAI*>& GetAliveEnemies() const;
    const TArray<AEnemyAI*>& GetDeadEnemies() const;

protected:

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    double BaselineSpawnInterval = 5.0;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double MinimumSpawnInterval = 0.5;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    int32 MaxSpawnIntervalIncreaseCount = 10;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnIntervalScale = 4.5;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnAccelerationRate = 2.0;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnIntervalIncreaseTimer = 30.0;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    int MaximumEnemies = 20;
    
    TMap<APlayerLocationDetection*, TArray<AEnemySpawner*>> SpawnersByLocation;
    TArray<AEnemySpawner*> CurrentEnemySpawners;
    TArray<AEnemySpawner*> CopyCurrentEnemySpawners;

    double UpdatedSpawnInterval;
    double SpawnInterval;
    int32 SpawnIntervalIncreaseCount;
    double SpawnIntervalIncreaseProgress;

    UPROPERTY()
    float RelocateDistanceThreshold = 10000.f;
    
    TArray<AEnemyAI*> AliveEnemies;
    TArray<AEnemyAI*> DeadEnemies;

    FTimerDelegate OutOfRangeDelegate;
    FTimerHandle OutOfRangeCheckTimer;
    float RangeCheckTimerInterval = 1.f;

private:
    AEnemySpawner* ChooseRandomSpawner();

    UFUNCTION()
    void CheckOutOfRange();
    
    void RelocateEnemy(AEnemyAI* Enemy);
    void BindPlayerLocationDetection(const UWorld::FActorsInitializedParams& Params);
    
    void OnEnterTriggerBox(APlayerLocationDetection* SpawnBox);
    void OnExitTriggerBox(APlayerLocationDetection* SpawnBox);
    static float CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent);
    void SpawnEnemy();
};