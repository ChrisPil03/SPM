#include "EnemySpawnManagerSubsystem.h"
#include "AIController.h"
#include "EnemySpawner.h"
#include "EnemyAI.h"
#include "EnemyAIController.h"
#include "EnemySpawnManagerSettings.h"
#include "PlayerLocationDetection.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EnemySpawnConfigurationDataAsset.h"
#include "BehaviorTree/BlackboardComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemySpawnSub, Log, All);

TStatId UEnemySpawnManagerSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UEnemySpawnSubsystem, STATGROUP_Tickables);
}

void UEnemySpawnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    const UEnemySpawnManagerSettings* Settings = UEnemySpawnManagerSettings::Get();
    if (Settings && Settings->SpawnConfigurationDataAssetPath.IsValid())
    {
        UEnemySpawnConfigurationDataAsset* ConfigData = Cast<UEnemySpawnConfigurationDataAsset>(Settings->SpawnConfigurationDataAssetPath.TryLoad());
        if (ConfigData)
        {
            ApplySpawnConfiguration(ConfigData);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load Spawn Configuration Data Asset from Project Settings path: %s"), *Settings->SpawnConfigurationDataAssetPath.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnConfigurationDataAssetPath is not set in EnemySpawnManagerSettings or settings are invalid."));
    }
    UpdatedSpawnInterval = BaselineSpawnInterval;
    SpawnInterval = BaselineSpawnInterval;
    SpawnIntervalIncreaseCount = 0;
    SpawnIntervalIncreaseProgress = SpawnIntervalIncreaseTimer;
    OutOfRangeDelegate.BindUFunction(this, FName("CheckOutOfRange"));
    GetWorld()->GetTimerManager().SetTimer(OutOfRangeCheckTimer, OutOfRangeDelegate, RangeCheckTimerInterval, true);
    AliveEnemiesByTypeMap.Empty();
    DeadEnemiesByTypeMap.Empty();
    SpawnersByLocation.Empty();
    CurrentEnemySpawners.Empty();

    FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UEnemySpawnManagerSubsystem::BindPlayerLocationDetection);
    
    UE_LOG(LogEnemySpawnSub, Log, TEXT("EnemySpawnSubsystem Initialized."));
}

TSubclassOf<AEnemyAI> UEnemySpawnManagerSubsystem::GetRandomAvailableEnemyTypeToSpawn() const
{
    TArray<TSubclassOf<AEnemyAI>> AvailableEnemyClasses;
    CurrentSpawnersByType.GetKeys(AvailableEnemyClasses);
    for (const TPair<TSubclassOf<AEnemyAI>, int32>& Pair : MaxEnemyCounts)
    {
        TSubclassOf<AEnemyAI> EnemyClass = Pair.Key;
        int32 MaxCountForThisType = Pair.Value;
        int32 CurrentCountForThisType = 0;

        const FEnemyArrayWrapper* EnemyWrapperPtr = AliveEnemiesByTypeMap.Find(EnemyClass);

        if (EnemyWrapperPtr)
        {
            CurrentCountForThisType = EnemyWrapperPtr->Enemies.Num();
        }
    
        if (CurrentCountForThisType >= MaxCountForThisType)
        {
            AvailableEnemyClasses.Remove(EnemyClass);
        }
        
    }
    if (AvailableEnemyClasses.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetRandomAvailableEnemyTypeToSpawn: No enemy types available to spawn (all might be at max capacity or none configured with capacity)."));
        return nullptr; // No available types
    }
    
    int32 RandomIndex = FMath::RandRange(0, AvailableEnemyClasses.Num() - 1);
    return AvailableEnemyClasses[RandomIndex];
}

void UEnemySpawnManagerSubsystem::ApplySpawnConfiguration(const UEnemySpawnConfigurationDataAsset* ConfigData)
{
    if (!ConfigData)
    {
        UE_LOG(LogTemp, Warning, TEXT("UEnemySpawnManagerSubsystem::ApplySpawnConfiguration: ConfigData is null. Cannot apply configuration."));
        return;
    }
    
    MaxEnemyCounts.Empty();
    
    for (const FEnemyTypeSpawnConfig& ConfigEntry : ConfigData->EnemyConfigs)
    {
        if (ConfigEntry.EnemyClass)
        {
            MaxEnemyCounts.Add(ConfigEntry.EnemyClass, ConfigEntry.MaxSpawnCount);
            MaximumEnemies += ConfigEntry.MaxSpawnCount;
            UE_LOG(LogTemp, Log, TEXT("Applied spawn config for %s: Max Count = %d"),
                   *ConfigEntry.EnemyClass->GetName(),
                   ConfigEntry.MaxSpawnCount);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UEnemySpawnManagerSubsystem::ApplySpawnConfiguration: Found an entry in ConfigData with no EnemyClass assigned. Skipping."));
        }
    }

    BaselineSpawnInterval           = ConfigData->BaselineSpawnInterval;
    MinimumSpawnInterval            = ConfigData->MinimumSpawnInterval;
    MaxSpawnIntervalIncreaseCount   = ConfigData->MaxSpawnIntervalIncreaseCount;
    SpawnIntervalScale              = ConfigData->SpawnIntervalScale;
    SpawnAccelerationRate           = ConfigData->SpawnAccelerationRate;
    SpawnIntervalIncreaseTimer      = ConfigData->SpawnIntervalIncreaseTimer;

    UE_LOG(LogTemp, Log, TEXT("Enemy spawn configuration applied. %d types configured."), MaxEnemyCounts.Num());
}

void UEnemySpawnManagerSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    SpawnInterval -= DeltaTime;
    if (SpawnInterval <= 0.f)
    {
        SpawnEnemy();
        SpawnInterval = UpdatedSpawnInterval;
    }

    SpawnIntervalIncreaseProgress -= DeltaTime;
    if (SpawnIntervalIncreaseProgress <= 0.f)
    {
        UpdatedSpawnInterval = CalculateSpawnTimer(SpawnIntervalIncreaseCount++, BaselineSpawnInterval, MinimumSpawnInterval, SpawnIntervalScale, MaxSpawnIntervalIncreaseCount, SpawnAccelerationRate);

        SpawnIntervalIncreaseProgress = SpawnIntervalIncreaseTimer;
    }
}

void UEnemySpawnManagerSubsystem::SpawnEnemy()
{
    int32 AliveEnemies = 0;
    for (const TTuple<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper> Pair : AliveEnemiesByTypeMap)
    {
        AliveEnemies += Pair.Value.Enemies.Num();
    }
    
    if (AliveEnemies >= MaximumEnemies)
    {
        return;
    }
    
    if (CurrentEnemySpawners.IsEmpty())
    {
        return;
    }
    
    TSubclassOf<AEnemyAI> EnemyClass = GetRandomAvailableEnemyTypeToSpawn();
    if (EnemyClass == nullptr)
    {
        return;
    }
    
    AEnemySpawner* RandomSpawner = ChooseRandomSpawner(EnemyClass);
    if (RandomSpawner == nullptr)
    {
        return;
    }
    
    AEnemyAI* Enemy = RandomSpawner->SpawnEnemy(EnemyClass);
    if (Enemy == nullptr)
    {
        return;
    }
    
    MarkEnemyAsAlive(Enemy);
}

void UEnemySpawnManagerSubsystem::RegisterSpawner(APlayerLocationDetection* SpawnLocation, AEnemySpawner* Spawner)
{
    if (SpawnLocation && Spawner)
    {
        SpawnersByLocation.FindOrAdd(SpawnLocation).AddUnique(Spawner);
    }
}

 void UEnemySpawnManagerSubsystem::MarkEnemyAsAlive(AEnemyAI* Enemy)
 {
    if (!IsValid(Enemy)) return;

    UClass* EnemyClass = Enemy->GetClass();
    if (!EnemyClass)  return; 

    FEnemyArrayWrapper& AliveEnemyListWrapper = AliveEnemiesByTypeMap.FindOrAdd(EnemyClass);

    if (!AliveEnemyListWrapper.Enemies.Contains(Enemy))
    {
        AliveEnemyListWrapper.Enemies.Add(Enemy);
        UE_LOG(LogTemp, Verbose, TEXT("Marked enemy '%s' (Type: %s) as alive. Total alive of this type: %d"),
            *Enemy->GetName(), *EnemyClass->GetName(), AliveEnemyListWrapper.Enemies.Num());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Enemy '%s' (Type: %s) was already marked as alive."),
            *Enemy->GetName(), *EnemyClass->GetName());
    }

    FEnemyArrayWrapper& DeadEnemyListWrapper = DeadEnemiesByTypeMap.FindOrAdd(EnemyClass);
    if (DeadEnemyListWrapper.Enemies.Contains(Enemy))
    {
        DeadEnemyListWrapper.Enemies.Remove(Enemy);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Tried to remove Enemy '%s' (Type: %s) from dead map but it was not marked as dead."),
            *Enemy->GetName(), *EnemyClass->GetName());
    }
    
 }

 void UEnemySpawnManagerSubsystem::MarkEnemyAsDead(AEnemyAI* Enemy)
{
    // --- Initial Validation ---
    if (!IsValid(Enemy))
    {
        UE_LOG(LogTemp, Warning, TEXT("MarkEnemyAsDead: Attempted to mark an INVALID or NULL enemy as dead. Function will exit."));
        return;
    }

    UClass* EnemyClass = Enemy->GetClass();
    if (!EnemyClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("MarkEnemyAsDead: Enemy '%s' has NO VALID CLASS. Cannot process. Function will exit."), *Enemy->GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("MarkEnemyAsDead: Processing enemy '%s' of type '%s'."), *Enemy->GetName(), *EnemyClass->GetName());

    // --- Add to DeadEnemiesByTypeMap ---
    UE_LOG(LogTemp, Verbose, TEXT("MarkEnemyAsDead: Accessing DeadEnemiesByTypeMap for class '%s'..."), *EnemyClass->GetName());
    FEnemyArrayWrapper& DeadEnemyListWrapper = DeadEnemiesByTypeMap.FindOrAdd(EnemyClass); // FindOrAdd is fine here

    int32 DeadCountBeforeAdd = DeadEnemyListWrapper.Enemies.Num();
    if (!DeadEnemyListWrapper.Enemies.Contains(Enemy))
    {
        DeadEnemyListWrapper.Enemies.Add(Enemy);
        UE_LOG(LogTemp, Log, TEXT("MarkEnemyAsDead: ADDED '%s' to dead list for type '%s'. Dead count for type: %d -> %d."),
            *Enemy->GetName(), *EnemyClass->GetName(), DeadCountBeforeAdd, DeadEnemyListWrapper.Enemies.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MarkEnemyAsDead: Enemy '%s' (Type: %s) was ALREADY in the dead list. No change to dead list. Current dead count for type: %d."),
            *Enemy->GetName(), *EnemyClass->GetName(), DeadCountBeforeAdd);
    }

    // --- Remove from AliveEnemiesByTypeMap ---
    UE_LOG(LogTemp, Verbose, TEXT("MarkEnemyAsDead: Accessing AliveEnemiesByTypeMap for class '%s'..."), *EnemyClass->GetName());

    // It's better to use Find here because if the class isn't in the alive map, we don't want to add an empty entry.
    if (FEnemyArrayWrapper* AliveEnemyListWrapperPtr = AliveEnemiesByTypeMap.Find(EnemyClass))
    {
        // The class type exists in the alive map, now check if this specific enemy instance is in its list.
        int32 AliveCountBeforeRemove = AliveEnemyListWrapperPtr->Enemies.Num();
        int32 RemovedCount = AliveEnemyListWrapperPtr->Enemies.Remove(Enemy); // Remove returns the number of elements removed (0 or 1)

        if (RemovedCount > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("MarkEnemyAsDead: REMOVED '%s' from alive list for type '%s'. Alive count for type: %d -> %d."),
                *Enemy->GetName(), *EnemyClass->GetName(), AliveCountBeforeRemove, AliveEnemyListWrapperPtr->Enemies.Num());

            // Optional: Clean up the map entry if the list becomes empty
            if (AliveEnemyListWrapperPtr->Enemies.Num() == 0)
            {
                UE_LOG(LogTemp, Verbose, TEXT("MarkEnemyAsDead: Alive list for type '%s' is now empty. Removing class entry from AliveEnemiesByTypeMap."), *EnemyClass->GetName());
                // AliveEnemiesByTypeMap.Remove(EnemyClass); // Uncomment if you want to remove empty entries
            }
        }
        else
        {
            // This means the class type was in the alive map, but this specific enemy instance wasn't in its list.
            UE_LOG(LogTemp, Warning, TEXT("MarkEnemyAsDead: Enemy '%s' (Type: %s) was NOT FOUND in its class's alive list for removal. Alive count for type remains: %d."),
                *Enemy->GetName(), *EnemyClass->GetName(), AliveCountBeforeRemove);
        }
    }
    else
    {
        // This means the EnemyClass itself was not a key in AliveEnemiesByTypeMap.
        UE_LOG(LogTemp, Warning, TEXT("MarkEnemyAsDead: No alive list found for type '%s' when trying to remove '%s'. Enemy was likely never marked alive or already removed from alive list."),
            *EnemyClass->GetName(), *Enemy->GetName());
    }

    // --- Log Final Counts for Verification (Optional but helpful) ---
    int32 FinalAliveCountThisType = 0;
    if (const FEnemyArrayWrapper* FinalAliveWrapper = AliveEnemiesByTypeMap.Find(EnemyClass))
    {
        FinalAliveCountThisType = FinalAliveWrapper->Enemies.Num();
    }
    int32 FinalDeadCountThisType = 0; // DeadEnemyListWrapper is still valid here
    FinalDeadCountThisType = DeadEnemyListWrapper.Enemies.Num();

    UE_LOG(LogTemp, Log, TEXT("MarkEnemyAsDead: FINISHED processing '%s'. Final counts for type '%s' - Alive: %d, Dead: %d."),
        *Enemy->GetName(), *EnemyClass->GetName(), FinalAliveCountThisType, FinalDeadCountThisType);
}

AEnemySpawner* UEnemySpawnManagerSubsystem::ChooseRandomSpawner(const TSubclassOf<AEnemyAI>& EnemyClassToSpawn)
{
    if (CurrentSpawnersByType[EnemyClassToSpawn].Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CurrentSpawnersByType[EnemyClassToSpawn].Num() - 1);
        return CurrentSpawnersByType[EnemyClassToSpawn][RandomIndex];
    }
    return nullptr;
}

void UEnemySpawnManagerSubsystem::RelocateToRandomSpawner(AEnemyAI* Enemy)
{
    if (Enemy == nullptr || Enemy->IsDead())
    {
        return;
    }
    
    if (AEnemySpawner* ChosenSpawner = ChooseRandomSpawner(Enemy->GetClass()))
    {
        AEnemyAIController* AIController = Cast<AEnemyAIController>(Enemy->GetController());
        AIController->BrainComponent->StopLogic("Relocating");
        ChosenSpawner->RelocateEnemy(Enemy);
        AIController->BrainComponent->GetBlackboardComponent()->InitializeBlackboard(*(Enemy->GetBehaviorTree()->BlackboardAsset));
        AIController->BrainComponent->StartLogic();
    }
}

void UEnemySpawnManagerSubsystem::CheckOutOfRange()
{
    if (AliveEnemiesByTypeMap.IsEmpty())
    {
        return;
    }
    
    for (const TTuple<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper> Pair : AliveEnemiesByTypeMap)
    {
        for (AEnemyAI* Enemy : Pair.Value.Enemies)
        {
            if (!IsValid(Enemy) || !IsValid(Enemy->GetController())) // Also check for valid controller
            {
                continue; // Skip invalid enemies or those without controllers
            }
            
            if (Cast<APlayerCharacter>(Enemy->GetTarget().GetObject()) == nullptr)
            {
                continue;
            }
            
            FName DistanceSqKey = "DistanceToTargetSquared";
            AAIController* EnemyController = Cast<AAIController>(Enemy->GetController());
            
            if (EnemyController)
            {
                UBlackboardComponent* Blackboard = EnemyController->GetBlackboardComponent();

                if (Blackboard)
                {
                    float DistanceSq =  Blackboard->GetValueAsFloat(DistanceSqKey);

                    if (DistanceSq > RelocateDistanceThreshold)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Distance is: %f"), DistanceSq)
                        RelocateToRandomSpawner(Enemy);
                    }
                }
            }
        }
    }
}

void UEnemySpawnManagerSubsystem::BindPlayerLocationDetection(const UWorld::FActorsInitializedParams& Params)
{
    TArray<AActor*> FoundLocations;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerLocationDetection::StaticClass(), FoundLocations);
    
    for (AActor* FoundActor : FoundLocations)
    {
        if (APlayerLocationDetection* PlayerLocationDetection = Cast<APlayerLocationDetection>(FoundActor))
        {
            PlayerLocationDetection->AddOnTriggerEnterFunction(this, &UEnemySpawnManagerSubsystem::OnEnterTriggerBox);
            PlayerLocationDetection->AddOnTriggerExitFunction(this, &UEnemySpawnManagerSubsystem::OnExitTriggerBox);
        }
    }
}

void UEnemySpawnManagerSubsystem::OnEnterTriggerBox(APlayerLocationDetection* SpawnBox)
{
    if (!IsValid(SpawnBox))
    {
        return;
    }

    TArray<AEnemySpawner*>* SpawnerArrayPtr = SpawnersByLocation.Find(SpawnBox);
    if (SpawnerArrayPtr != nullptr)
    {
        TArray<AEnemySpawner*>& SpawnerArray = *SpawnerArrayPtr;
        for (AEnemySpawner* Spawner : SpawnerArray)
        {
            if (IsValid(Spawner))
            {
                CurrentEnemySpawners.AddUnique(Spawner);

                for (TSubclassOf<AEnemyAI> EnemyClass : Spawner->GetSpawnableEnemies())
                {
                    CurrentSpawnersByType.FindOrAdd(EnemyClass).AddUnique(Spawner);
                }
            }
            else
            {
                UE_LOG(LogEnemySpawnSub, Warning, TEXT("Found an invalid Spawner pointer associated with SpawnBox %s."), *SpawnBox->GetName());
            }
        }
    }
    else
    {
        UE_LOG(LogEnemySpawnSub, Warning, TEXT("OnEnterTriggerBox: SpawnBox '%s' not found as a key in SpawnersByLocation map. No spawners added."), *SpawnBox->GetName());
    }
}

void UEnemySpawnManagerSubsystem::OnExitTriggerBox(APlayerLocationDetection* SpawnBox)
{
    if (!IsValid(SpawnBox))
    {
        UE_LOG(LogEnemySpawnSub, Warning, TEXT("OnExitTriggerBox called with invalid SpawnBox pointer."));
        return;
    }

    TArray<AEnemySpawner*>* SpawnerArrayPtr = SpawnersByLocation.Find(SpawnBox);

    if (SpawnerArrayPtr != nullptr)
    {
        TArray<AEnemySpawner*>& SpawnerArray = *SpawnerArrayPtr;
        for (AEnemySpawner* Spawner : SpawnerArray)
        {
            CurrentEnemySpawners.Remove(Spawner);
        }

        for (AEnemySpawner* Spawner : SpawnerArray)
        {
            if (!Spawner)
            {
                continue;
            }
            
            for (auto It = CurrentSpawnersByType.CreateIterator(); It; ++It)
            {
                if (It.Value().Contains(Spawner))
                {
                    It.Value().Remove(Spawner);
                }

                if (It.Value().IsEmpty())
                {
                    It.RemoveCurrent();
                }
            }
        }
    }
    else
    {
        UE_LOG(LogEnemySpawnSub, Warning, TEXT("OnExitTriggerBox: SpawnBox '%s' not found as a key in SpawnersByLocation map. No spawners removed."), *SpawnBox->GetName());
    }
}

const TMap<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper>& UEnemySpawnManagerSubsystem::GetAliveEnemiesMap() const 
{
    return AliveEnemiesByTypeMap;
}

const TMap<TSubclassOf<AEnemyAI>, FEnemyArrayWrapper>& UEnemySpawnManagerSubsystem::GetDeadEnemiesMap() const 
{
    return DeadEnemiesByTypeMap;
}

TArray<AEnemyAI*> UEnemySpawnManagerSubsystem::GetAliveEnemiesByType(const TSubclassOf<AEnemyAI>& EnemyClass) const
{
    return AliveEnemiesByTypeMap[EnemyClass].Enemies;
}

int32 UEnemySpawnManagerSubsystem::GetMaxEnemiesByType(const TSubclassOf<AEnemyAI>& EnemyClass) const
{
    return MaxEnemyCounts[EnemyClass];
}

float UEnemySpawnManagerSubsystem::CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent)
{

  float tNorm = (maxCycles <= 0) ? 0.0f : FMath::Clamp(float(cycleIndex) / float(maxCycles), 0.0f, 1.0f);

  float deltaNorm = FMath::Pow(tNorm, exponent);

  float t = baselineInterval
          - intervalScale * deltaNorm;

  return FMath::Max(minimumInterval, t);
}
