#include "EnemySpawnManagerSubsystem.h"

#include "AIController.h"
#include "EnemySpawner.h"
#include "EnemyAI.h"
#include "PlayerLocationDetection.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "BehaviorTree/BlackboardComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemySpawnSub, Log, All);

TStatId UEnemySpawnManagerSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UEnemySpawnSubsystem, STATGROUP_Tickables);
}

void UEnemySpawnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
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
    CopyCurrentEnemySpawners.Empty();

    FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UEnemySpawnManagerSubsystem::BindPlayerLocationDetection);
    
    UE_LOG(LogEnemySpawnSub, Log, TEXT("EnemySpawnSubsystem Initialized."));
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
    if (CopyCurrentEnemySpawners.IsEmpty())
    {
        CopyCurrentEnemySpawners = CurrentEnemySpawners;
    }
    
    if (CopyCurrentEnemySpawners.IsEmpty())
    {
        return;
    }

    AEnemyAI* Enemy = ChooseRandomSpawner()->SpawnEnemy();
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
    if (CopyCurrentEnemySpawners.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CopyCurrentEnemySpawners.Num() - 1);
        AEnemySpawner* ChosenSpawner = CopyCurrentEnemySpawners[RandomIndex];
        CopyCurrentEnemySpawners.RemoveAt(RandomIndex);
        return ChosenSpawner;
    }
    return nullptr;
}

void UEnemySpawnManagerSubsystem::CheckOutOfRange()
{
    for (AEnemyAI* Enemy : AliveEnemies)
    {
        FName DistanceSqKey = "DistanceToTargetSquared";
        float DistanceSq = Cast<AAIController>(Enemy->GetController())->GetBlackboardComponent()->GetValueAsFloat(DistanceSqKey);

        if (DistanceSq > RelocateDistanceThreshold)
        {
            RelocateEnemy(Enemy);
        }
    }
}

void UEnemySpawnManagerSubsystem::RelocateEnemy(AEnemyAI* Enemy)
{
    ChooseRandomSpawner()->RelocateEnemy(Enemy);
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
    for (AEnemySpawner* Spawner : *SpawnersByLocation.Find(SpawnBox))
    {
        CurrentEnemySpawners.Add(Spawner);
    }
    CopyCurrentEnemySpawners.Empty();
}

void UEnemySpawnManagerSubsystem::OnExitTriggerBox(APlayerLocationDetection* SpawnBox)
{
    for (AEnemySpawner* Spawner : *SpawnersByLocation.Find(SpawnBox))
    {
        CurrentEnemySpawners.Remove(Spawner);
    }
    CopyCurrentEnemySpawners.Empty();
}


const TArray<AEnemyAI*>& UEnemySpawnManagerSubsystem::GetAliveEnemies() const 
{
    return AliveEnemies;
}

const TArray<AEnemyAI*>& UEnemySpawnManagerSubsystem::GetDeadEnemies() const 
{
    return DeadEnemies;
}

float UEnemySpawnManagerSubsystem::CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent)
{

  float tNorm = (maxCycles <= 0) ? 0.0f : FMath::Clamp(float(cycleIndex) / float(maxCycles), 0.0f, 1.0f);

  float deltaNorm = FMath::Pow(tNorm, exponent);

  float t = baselineInterval
          - intervalScale * deltaNorm;

  return FMath::Max(minimumInterval, t);
}