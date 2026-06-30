#include "EnemySpawnManagerSubsystem.h"
#include "AIController.h"
#include "DiveGameMode.h"
#include "EnemySpawner.h"
#include "EnemyAI.h"
#include "EnemyAIController.h"
#include "EnemySpawnManagerSettings.h"
#include "PlayerLocationDetection.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EnemySpawnConfigurationDataAsset.h"
#include "ObjectiveDefendGenerator.h"
#include "ObjectiveManagerSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"

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
    MainObjectiveActive = false;

    OutOfRangeDelegate.BindUFunction(this, FName("CheckOutOfRange"));
    GetWorld()->GetTimerManager().SetTimer(OutOfRangeCheckTimer, OutOfRangeDelegate, RangeCheckTimerInterval, true);
    AliveEnemiesByTypeMap.Empty();
    DeadEnemiesByTypeMap.Empty();
    SpawnersByLocation.Empty();
    CurrentEnemySpawners.Empty();

    

    FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UEnemySpawnManagerSubsystem::FetchEnemySpawnerCount);
    FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UEnemySpawnManagerSubsystem::BindPlayerLocationDetection);
    
}


void UEnemySpawnManagerSubsystem::ChangeEnemySpawnersToMainObjective(AObjectiveBase* MainObjective)
{
    // UE_LOG(LogTemp, Warning, TEXT("ChangeEnemySpawnersToMainObjective"));

    MainObjectiveActive = true;
    
    CurrentEnemySpawners = MainObjectiveEnemySpawners;
    CurrentSpawnersByType = MainObjectiveSpawnersByType;
}

void UEnemySpawnManagerSubsystem::ChangeEnemySpawnersToPlayer(AObjectiveBase* MainObjective)
{
    // UE_LOG(LogTemp, Warning, TEXT("ChangeEnemySpawnersToPlayer"));

    MainObjectiveActive = false;
    
    CurrentEnemySpawners = PlayerEnemySpawners;
    CurrentSpawnersByType = PlayerSpawnersByType;
}

void UEnemySpawnManagerSubsystem::FetchEnemySpawnerCount(const UWorld::FActorsInitializedParams& Params)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AEnemySpawner::StaticClass(), FoundActors);

        // UE_LOG(LogTemp, Log, TEXT("Found %d total actors of AEnemySpawner class."), FoundActors.Num());

        for (AActor* Actor : FoundActors)
        {
            AEnemySpawner* Spawner = Cast<AEnemySpawner>(Actor);
            if (Spawner)
            {
                // UE_LOG(LogTemp, Log, TEXT("Processing Enemy Spawner: %s"), *Spawner->GetName());
                if (Spawner->GetConnectedPlayerLocationDetection()) { TotalSpawnersCount++; }
            }
        }
    }
}

void UEnemySpawnManagerSubsystem::RegisterMainObjectiveEnemySpawners()
{
    UObjectiveManagerSubsystem* ObjectiveManager = GetWorld()->GetSubsystem<UObjectiveManagerSubsystem>();
    if (!ObjectiveManager) return;

    AObjectiveDefendGenerator* MainObjective = ObjectiveManager->GetMainObjective();
    if (!MainObjective) return;

    APlayerLocationDetection* ObjectivePlayerDetection = MainObjective->GetObjectivePlayerDetection();
    if (!ObjectivePlayerDetection) return;

    TArray<AEnemySpawner*>* MainObjectiveSpawnerPtr = SpawnersByLocation.Find(ObjectivePlayerDetection);
    if (!MainObjectiveSpawnerPtr) return;
    
    MainObjectiveEnemySpawners = *MainObjectiveSpawnerPtr;


    for (AEnemySpawner* Spawner : MainObjectiveEnemySpawners)
    {
        if (IsValid(Spawner))
        {
            for (TSubclassOf<AEnemyAI> EnemyClass : Spawner->GetSpawnableEnemies())
            {
                MainObjectiveSpawnersByType.FindOrAdd(EnemyClass).AddUnique(Spawner);
            }
        }
    }
    MainObjective->AddOnObjectiveActivatedFunction(this, &UEnemySpawnManagerSubsystem::ChangeEnemySpawnersToMainObjective);
    MainObjective->AddOnObjectiveDeactivatedFunction(this, &UEnemySpawnManagerSubsystem::ChangeEnemySpawnersToPlayer);
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
        // UE_LOG(LogTemp, Warning, TEXT("GetRandomAvailableEnemyTypeToSpawn: No enemy types available to spawn (all might be at max capacity or none configured with capacity)."));
        return nullptr; // No available types
    }
    
    int32 RandomIndex = FMath::RandRange(0, AvailableEnemyClasses.Num() - 1);
    return AvailableEnemyClasses[RandomIndex];
}

void UEnemySpawnManagerSubsystem::ApplySpawnConfiguration(const UEnemySpawnConfigurationDataAsset* ConfigData)
{
    if (!ConfigData)
    {
        // UE_LOG(LogTemp, Warning, TEXT("UEnemySpawnManagerSubsystem::ApplySpawnConfiguration: ConfigData is null. Cannot apply configuration."));
        return;
    }
    
    MaxEnemyCounts.Empty();
    
    for (const FEnemyTypeSpawnConfig& ConfigEntry : ConfigData->EnemyConfigs)
    {
        if (ConfigEntry.EnemyClass)
        {
            MaxEnemyCounts.Add(ConfigEntry.EnemyClass, ConfigEntry.MaxSpawnCount);
            MaximumEnemies += ConfigEntry.MaxSpawnCount;
            // UE_LOG(LogTemp, Log, TEXT("Applied spawn config for %s: Max Count = %d"),
            //        *ConfigEntry.EnemyClass->GetName(),
            //        ConfigEntry.MaxSpawnCount);
        }
        else
        {
            // UE_LOG(LogTemp, Warning, TEXT("UEnemySpawnManagerSubsystem::ApplySpawnConfiguration: Found an entry in ConfigData with no EnemyClass assigned. Skipping."));
        }
    }

    // UE_LOG(LogTemp, Log, TEXT("Enemy spawn configuration applied. %d types configured."), MaxEnemyCounts.Num());
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
        CurrentSpawnersCount++;
        // UE_LOG(LogTemp, Warning, TEXT("Total EnemySpawners: %d\nCurrentSpawnersCount: %d"), TotalSpawnersCount, CurrentSpawnersCount)
        if (CurrentSpawnersCount == TotalSpawnersCount)
        {
            RegisterMainObjectiveEnemySpawners();
        }
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
        // UE_LOG(LogTemp, Verbose, TEXT("Marked enemy '%s' (Type: %s) as alive. Total alive of this type: %d"),
        //     *Enemy->GetName(), *EnemyClass->GetName(), AliveEnemyListWrapper.Enemies.Num());
    }
    else
    {
        // UE_LOG(LogTemp, Log, TEXT("Enemy '%s' (Type: %s) was already marked as alive."),
        //     *Enemy->GetName(), *EnemyClass->GetName());
    }

    FEnemyArrayWrapper& DeadEnemyListWrapper = DeadEnemiesByTypeMap.FindOrAdd(EnemyClass);
    if (DeadEnemyListWrapper.Enemies.Contains(Enemy))
    {
        DeadEnemyListWrapper.Enemies.Remove(Enemy);
    }
    else
    {
        // UE_LOG(LogTemp, Log, TEXT("Tried to remove Enemy '%s' (Type: %s) from dead map but it was not marked as dead."),
        //     *Enemy->GetName(), *EnemyClass->GetName());
    }
    
 }

 void UEnemySpawnManagerSubsystem::MarkEnemyAsDead(AEnemyAI* Enemy)
{
    if (!IsValid(Enemy))
    {
        return;
    }

    UClass* EnemyClass = Enemy->GetClass();
    if (!EnemyClass)
    {
        return;
    }
    
    FEnemyArrayWrapper& DeadEnemyListWrapper = DeadEnemiesByTypeMap.FindOrAdd(EnemyClass);

    int32 DeadCountBeforeAdd = DeadEnemyListWrapper.Enemies.Num();
    if (!DeadEnemyListWrapper.Enemies.Contains(Enemy))
    {
        DeadEnemyListWrapper.Enemies.Add(Enemy);
    }

    if (FEnemyArrayWrapper* AliveEnemyListWrapperPtr = AliveEnemiesByTypeMap.Find(EnemyClass))
    {
        int32 AliveCountBeforeRemove = AliveEnemyListWrapperPtr->Enemies.Num();
        int32 RemovedCount = AliveEnemyListWrapperPtr->Enemies.Remove(Enemy);

    }
    
    int32 FinalAliveCountThisType = 0;
    if (const FEnemyArrayWrapper* FinalAliveWrapper = AliveEnemiesByTypeMap.Find(EnemyClass))
    {
        FinalAliveCountThisType = FinalAliveWrapper->Enemies.Num();
    }
    int32 FinalDeadCountThisType = 0;
    FinalDeadCountThisType = DeadEnemyListWrapper.Enemies.Num();
}

AEnemySpawner* UEnemySpawnManagerSubsystem::ChooseRandomSpawner(const TSubclassOf<AEnemyAI>& EnemyClassToSpawn)
{
    TArray<AEnemySpawner*>* SpawnersPtr = CurrentSpawnersByType.Find(EnemyClassToSpawn);
    if (!SpawnersPtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Something went wrong with choosing a random spawner"))
        return nullptr;
    }
    
    if (CurrentSpawnersByType.Find(EnemyClassToSpawn)->Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CurrentSpawnersByType.Find(EnemyClassToSpawn)->Num() - 1);
        return (*CurrentSpawnersByType.Find(EnemyClassToSpawn))[RandomIndex];
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
        ChosenSpawner->RelocateEnemy(Enemy);
        AIController->BrainComponent->GetBlackboardComponent()->SetValueAsBool("HasBeenRelocated", true);
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
            if (!IsValid(Enemy) || !IsValid(Enemy->GetController()))
            {
                continue;
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
                        RelocateToRandomSpawner(Enemy);
                    }
                }
            }
        }
    }
}

void UEnemySpawnManagerSubsystem::BindPlayerLocationDetection(const UWorld::FActorsInitializedParams& Params)
{
    GameMode = Cast<ADiveGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
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
                PlayerEnemySpawners.AddUnique(Spawner);
                
                for (TSubclassOf<AEnemyAI> EnemyClass : Spawner->GetSpawnableEnemies())
                {
                    PlayerSpawnersByType.FindOrAdd(EnemyClass).AddUnique(Spawner);
                }

                if (!MainObjectiveActive)
                {
                    CurrentEnemySpawners.AddUnique(Spawner);
                    CurrentSpawnersByType = PlayerSpawnersByType;
                }
            }
        }
    }
}

void UEnemySpawnManagerSubsystem::OnExitTriggerBox(APlayerLocationDetection* SpawnBox)
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
            PlayerEnemySpawners.Remove(Spawner);
        }

        for (AEnemySpawner* Spawner : SpawnerArray)
        {
            if (!Spawner)
            {
                continue;
            }
            
            for (auto It = PlayerSpawnersByType.CreateIterator(); It; ++It)
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

        if (!MainObjectiveActive)
        {
            CurrentEnemySpawners = PlayerEnemySpawners;
            CurrentSpawnersByType = PlayerSpawnersByType;
        }
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
    return AliveEnemiesByTypeMap.Find(EnemyClass)->Enemies;
}

int32 UEnemySpawnManagerSubsystem::GetMaxEnemiesByType(const TSubclassOf<AEnemyAI>& EnemyClass) const
{
    return (*MaxEnemyCounts.Find(EnemyClass));
}