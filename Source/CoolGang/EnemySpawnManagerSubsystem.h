
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemySpawnManagerSubsystem.generated.h"

class UEnemySpawnConfigurationDataAsset;
// Forward declarations
class APlayerLocationDetection;
class AEnemySpawner;
class AEnemyAI;

USTRUCT() 
struct FEnemyArrayWrapper
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<AEnemyAI*> Enemies;
    
    FEnemyArrayWrapper() {}
    
    FEnemyArrayWrapper(const TArray<AEnemyAI*>& InEnemies) : Enemies(InEnemies) {}
};

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

    /** Gets the current number of spawned enemies of a specific Blueprint class. */
    UFUNCTION(BlueprintPure, Category = "Enemy Spawn Manager")
    int32 GetSpawnedEnemyCountByType(TSubclassOf<AEnemyAI> EnemyClass) const;

    /** Gets all spawned enemies of a specific Blueprint class. */
    UFUNCTION(BlueprintPure, Category = "Enemy Spawn Manager")
    TArray<AEnemyAI*> GetSpawnedEnemiesByType(TSubclassOf<AEnemyAI> EnemyClass) const;

protected:

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    double BaselineSpawnInterval = 4.0;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double MinimumSpawnInterval = 0.4;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    int32 MaxSpawnIntervalIncreaseCount = 15;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnIntervalScale = 2.5;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnAccelerationRate = 3.4;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnIntervalIncreaseTimer = 20.0;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    int MaximumEnemies = 20;
    
    TMap<APlayerLocationDetection*, TArray<AEnemySpawner*>> SpawnersByLocation;
    TArray<AEnemySpawner*> CurrentEnemySpawners;

    double UpdatedSpawnInterval;
    double SpawnInterval;
    int32 SpawnIntervalIncreaseCount;
    double SpawnIntervalIncreaseProgress;

    UPROPERTY()
    float RelocateDistanceThreshold = 10000.f * 10000.f;
    
    TArray<AEnemyAI*> AliveEnemies;
    TArray<AEnemyAI*> DeadEnemies;

    FTimerDelegate OutOfRangeDelegate;
    FTimerHandle OutOfRangeCheckTimer;
    float RangeCheckTimerInterval = 1.f;

private:
    UPROPERTY(VisibleInstanceOnly, Category = "Enemy Spawn Manager|Limits")
    TMap<TSubclassOf<AEnemyAI>, int32> MaxEnemyCounts;
    
    UPROPERTY(VisibleInstanceOnly, Category = "Enemy Spawn Manager|Runtime")
    TMap<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper> SpawnedEnemiesByTypeMap;

    void ApplySpawnConfiguration(const UEnemySpawnConfigurationDataAsset* ConfigData);
    
    UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
    void RelocateToRandomSpawner(AEnemyAI* Enemy);
    
    AEnemySpawner* ChooseRandomSpawner();
    
    UFUNCTION()
    void CheckOutOfRange();

    void BindPlayerLocationDetection(const UWorld::FActorsInitializedParams& Params);
    
    void OnEnterTriggerBox(APlayerLocationDetection* SpawnBox);
    void OnExitTriggerBox(APlayerLocationDetection* SpawnBox);
    static float CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent);
    void SpawnEnemy();
};