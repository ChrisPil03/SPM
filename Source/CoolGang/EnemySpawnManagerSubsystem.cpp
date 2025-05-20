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
#include "EnemySpawnDataTypes.h"
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
    AliveEnemies.Empty();
    DeadEnemies.Empty();
    SpawnersByLocation.Empty();
    CurrentEnemySpawners.Empty();

    FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UEnemySpawnManagerSubsystem::BindPlayerLocationDetection);
    
    UE_LOG(LogEnemySpawnSub, Log, TEXT("EnemySpawnSubsystem Initialized."));
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

            UE_LOG(LogTemp, Log, TEXT("Applied spawn config for %s: Max Count = %d"),
                   *ConfigEntry.EnemyClass->GetName(),
                   ConfigEntry.MaxSpawnCount);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UEnemySpawnManagerSubsystem::ApplySpawnConfiguration: Found an entry in ConfigData with no EnemyClass assigned. Skipping."));
        }
    }

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
    if (AliveEnemies.Num() >= MaximumEnemies)
    {
        return;
    }

    if (CurrentEnemySpawners.IsEmpty())
    {
        return;
    }

    AEnemySpawner* RandomSpawner = ChooseRandomSpawner();
    if (RandomSpawner == nullptr)
    {
        return;
    }
    
    AEnemyAI* Enemy = RandomSpawner->SpawnEnemy();
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
    if (!Enemy) return;

    DeadEnemies.Remove(Enemy);
    AliveEnemies.AddUnique(Enemy);
 }

 void UEnemySpawnManagerSubsystem::MarkEnemyAsDead(AEnemyAI* Enemy)
 {
    if (!Enemy) return;

    if (AliveEnemies.Contains(Enemy))
    {
        AliveEnemies.Remove(Enemy);
        DeadEnemies.AddUnique(Enemy);
    }
 }

AEnemySpawner* UEnemySpawnManagerSubsystem::ChooseRandomSpawner()
{
    if (CurrentEnemySpawners.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CurrentEnemySpawners.Num() - 1);
        return CurrentEnemySpawners[RandomIndex];
    }
    return nullptr;
}

void UEnemySpawnManagerSubsystem::RelocateToRandomSpawner(AEnemyAI* Enemy)
{
    if (Enemy == nullptr)
    {
        return;
    }
    
    if (AEnemySpawner* ChosenSpawner = ChooseRandomSpawner())
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
    if (AliveEnemies.Num() == 0)
    {
        return;
    }
    for (AEnemyAI* Enemy : AliveEnemies)
    {
        if (Cast<APlayerCharacter>(Enemy->GetTarget().GetObject()) == nullptr)
        {
            return;
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
    }
    else
    {
        UE_LOG(LogEnemySpawnSub, Warning, TEXT("OnExitTriggerBox: SpawnBox '%s' not found as a key in SpawnersByLocation map. No spawners removed."), *SpawnBox->GetName());
    }
}


const TArray<AEnemyAI*>& UEnemySpawnManagerSubsystem::GetAliveEnemies() const 
{
    return AliveEnemies;
}

const TArray<AEnemyAI*>& UEnemySpawnManagerSubsystem::GetDeadEnemies() const 
{
    return DeadEnemies;
}

int32 UEnemySpawnManagerSubsystem::GetSpawnedEnemyCountByType(TSubclassOf<AEnemyAI> EnemyClass) const
{
    return MaxEnemyCounts[EnemyClass];
}

TArray<AEnemyAI*> UEnemySpawnManagerSubsystem::GetSpawnedEnemiesByType(TSubclassOf<AEnemyAI> EnemyClass) const
{
    return SpawnedEnemiesByTypeMap[EnemyClass].Enemies;
}

float UEnemySpawnManagerSubsystem::CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent)
{

  float tNorm = (maxCycles <= 0) ? 0.0f : FMath::Clamp(float(cycleIndex) / float(maxCycles), 0.0f, 1.0f);

  float deltaNorm = FMath::Pow(tNorm, exponent);

  float t = baselineInterval
          - intervalScale * deltaNorm;

  return FMath::Max(minimumInterval, t);
}
