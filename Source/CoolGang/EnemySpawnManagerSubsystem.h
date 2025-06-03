
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemySpawnManagerSubsystem.generated.h"

class UEnemySpawnConfigurationDataAsset;
// Forward declarations
class APlayerLocationDetection;
class AEnemySpawner;
class AEnemyAI;
class AObjectiveBase;

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
    const TMap<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper>& GetAliveEnemiesMap() const;
    const TMap<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper>& GetDeadEnemiesMap() const;
    
    /** Gets all spawned enemies of a specific Blueprint class. */
    UFUNCTION(BlueprintPure, Category = "Enemy Spawn Manager")
    TArray<AEnemyAI*> GetAliveEnemiesByType(const TSubclassOf<AEnemyAI>& EnemyClass) const;

    UFUNCTION(BlueprintPure, Category = "Enemy Spawn Manager")
    int32 GetMaxEnemiesByType(const TSubclassOf<AEnemyAI>& EnemyClass) const;

    
protected:

    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    double BaselineSpawnInterval = 4.0;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double MinimumSpawnInterval = 0.2;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    int32 MaxSpawnIntervalIncreaseCount = 20;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnIntervalScale = 2.5;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnAccelerationRate = 3.2;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    double SpawnIntervalIncreaseTimer = 20.0;
    
    TMap<APlayerLocationDetection*, TArray<AEnemySpawner*>> SpawnersByLocation;
    TMap<TSubclassOf<AEnemyAI>, TArray<AEnemySpawner*>> CurrentSpawnersByType;
    TMap<TSubclassOf<AEnemyAI>, TArray<AEnemySpawner*>> MainObjectiveSpawnersByType;
    TMap<TSubclassOf<AEnemyAI>, TArray<AEnemySpawner*>> PlayerSpawnersByType;
    TArray<AEnemySpawner*> CurrentEnemySpawners;
    TArray<AEnemySpawner*> PlayerEnemySpawners;
    TArray<AEnemySpawner*> MainObjectiveEnemySpawners;
    bool MainObjectiveActive = false;

    double UpdatedSpawnInterval;
    double SpawnInterval;
    int32 SpawnIntervalIncreaseCount;
    double SpawnIntervalIncreaseProgress;

    UPROPERTY()
    float RelocateDistanceThreshold = 6500.f * 6500.f;

    FTimerDelegate OutOfRangeDelegate;
    FTimerHandle OutOfRangeCheckTimer;
    float RangeCheckTimerInterval = 1.f;

private:

    void ChangeEnemySpawnersToMainObjective(AObjectiveBase* MainObjective);

    void ChangeEnemySpawnersToPlayer(AObjectiveBase* MainObjective);

    void FetchEnemySpawnerCount(const UWorld::FActorsInitializedParams& Params);

    void RegisterMainObjectiveEnemySpawners();
    
    TSubclassOf<AEnemyAI> GetRandomAvailableEnemyTypeToSpawn() const;
    
    UPROPERTY(VisibleInstanceOnly, Category = "Enemy Spawn Manager|Limits")
    TMap<TSubclassOf<AEnemyAI>, int32> MaxEnemyCounts;

    int32 MaximumEnemies;

    int32 TotalSpawnersCount = 0;
    int32 CurrentSpawnersCount = 0;
    
    UPROPERTY(VisibleInstanceOnly, Category = "Enemy Spawn Manager|Runtime")
    TMap<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper> AliveEnemiesByTypeMap;

    UPROPERTY(VisibleInstanceOnly, Category = "Enemy Spawn Manager|Runtime")
    TMap<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper> DeadEnemiesByTypeMap;

    void ApplySpawnConfiguration(const UEnemySpawnConfigurationDataAsset* ConfigData);
    
    UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
    void RelocateToRandomSpawner(AEnemyAI* Enemy);
    
    AEnemySpawner* ChooseRandomSpawner(const TSubclassOf<AEnemyAI>& EnemyClassToSpawn);
    
    UFUNCTION()
    void CheckOutOfRange();

    void BindPlayerLocationDetection(const UWorld::FActorsInitializedParams& Params);
    
    void OnEnterTriggerBox(APlayerLocationDetection* SpawnBox);
    void OnExitTriggerBox(APlayerLocationDetection* SpawnBox);
    static float CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent);
    void SpawnEnemy();
};