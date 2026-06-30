#include "BTService_TargetLocationGround.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "EnemyAI.h"
#include "Attackable.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavFilters/NavigationQueryFilter.h"

UBTService_TargetLocationGround::UBTService_TargetLocationGround()
{
    NodeName = "Update Target Location On Ground";

    bPreferNavMeshProjectionForAirborneTarget = true; 
    NavMeshProjectionVerticalThreshold = 500.0f;
    NavMeshProjectionHorizontalExtent = 500.0f;

    bApplyRandomOffset = true;
    MaxRandomOffsetRadius = 300.0f;

    GroundTraceDistance = 100.0f;
    GroundTraceChannel = ECC_Visibility;
    bDrawDebugTraceForDuration = false;
    DebugTraceDuration = 2.0f;
}

void UBTService_TargetLocationGround::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_TickTotal")); // Profile the entire TickNode function

    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return;

    AAIController* OwnerController = OwnerComp.GetAIOwner();
    if (!OwnerController) { BlackboardComp->ClearValue(GetSelectedBlackboardKey()); return; }

    APawn* OwnerPawn = OwnerController->GetPawn();
    if (!OwnerPawn) { BlackboardComp->ClearValue(GetSelectedBlackboardKey()); return; }

    AEnemyAI* EnemyAI = Cast<AEnemyAI>(OwnerPawn);
    if (!EnemyAI) { BlackboardComp->ClearValue(GetSelectedBlackboardKey()); return; }

    TScriptInterface<IAttackable> TargetInterface = EnemyAI->GetTarget();
    if (!TargetInterface.GetInterface()) { BlackboardComp->ClearValue(GetSelectedBlackboardKey()); return; }

    AActor* TargetActor = Cast<AActor>(TargetInterface.GetObject());
    if (!TargetActor) { BlackboardComp->ClearValue(GetSelectedBlackboardKey()); return; }

    UWorld* World = OwnerPawn->GetWorld();
    if (!World) { BlackboardComp->ClearValue(GetSelectedBlackboardKey()); return; }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    ANavigationData* DefaultNavData = nullptr;
    if (NavSys)
    {
        DefaultNavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    }

    FVector BaseTargetLocation;
    bool bBaseTargetFound = false;
    
    FHitResult HitResult;
    FNavLocation ProjectedGroundPoint, ProjectedNodeLocation, ProjectedImpactLocation, ProjectedNodeLoc;

    if (bPreferNavMeshProjectionForAirborneTarget && NavSys && DefaultNavData)
    {
        bool bTargetIsLikelyOffNavMesh = true; 
        {
            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_AirborneTargetCheckLogic"));
            ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
            if (TargetCharacter && TargetCharacter->GetCharacterMovement() &&
                TargetCharacter->GetCharacterMovement()->IsMovingOnGround() &&
                !TargetCharacter->GetCharacterMovement()->IsFalling())
            {
                bTargetIsLikelyOffNavMesh = false; 
            }
        }

        if (bTargetIsLikelyOffNavMesh) 
        {
            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_AirbornePath_MovementNodeProcessing"));
            TArray<USphereComponent*> MovementNodes = TargetInterface->GetMovementNodes();
            if (MovementNodes.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, MovementNodes.Num() - 1);
                USphereComponent* SelectedNode = MovementNodes.IsValidIndex(RandomIndex) ? MovementNodes[RandomIndex] : nullptr;

                if (SelectedNode)
                {
                    FVector NodeLocation = SelectedNode->GetComponentLocation();
                    FCollisionQueryParams CollisionParams;
                    CollisionParams.AddIgnoredActor(OwnerPawn);
                    CollisionParams.AddIgnoredActor(TargetActor);

                    FVector StartTrace = NodeLocation;
                    FVector EndTrace = NodeLocation - FVector(0.f, 0.f, GroundTraceDistance + SelectedNode->GetScaledSphereRadius());
                    
                    bool bGroundTraceSucceeded;
                    {
                        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_LineTrace_GroundSearch"));
                        bGroundTraceSucceeded = World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, GroundTraceChannel, CollisionParams);
                    }
                     if (bDrawDebugTraceForDuration)
                    {
                        DrawDebugLine(World, StartTrace, EndTrace, FColor::Green, false, DebugTraceDuration, 0, 1.f);
                    }

                    if (bGroundTraceSucceeded)
                    {
                        FVector QueryExtent(NavMeshProjectionHorizontalExtent * 0.25f, NavMeshProjectionHorizontalExtent * 0.25f, NavMeshProjectionVerticalThreshold);
                        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
                        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);
                        
                        bool bNavProjectionSucceeded;
                        {
                            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_NavMeshProjection_Call"));
                            bNavProjectionSucceeded = NavSys->ProjectPointToNavigation(HitResult.ImpactPoint, ProjectedGroundPoint, QueryExtent, DefaultNavData, NavQueryFilter);
                        }
                        if (bNavProjectionSucceeded)
                        {
                            BaseTargetLocation = ProjectedGroundPoint.Location;
                            bBaseTargetFound = true;
                            if (bDrawDebugTraceForDuration)
                            {
                                DrawDebugSphere(World, HitResult.ImpactPoint, 10.f, 12, FColor::Red, false, DebugTraceDuration, 0, 1.5f); 
                                DrawDebugSphere(World, BaseTargetLocation, 15.f, 12, FColor::Cyan, false, DebugTraceDuration, 0, 2.f); 
                            }
                        }
                    }
                    else
                    {
                        FVector QueryExtent(NavMeshProjectionHorizontalExtent * 0.5f, NavMeshProjectionHorizontalExtent * 0.5f, NavMeshProjectionVerticalThreshold);
                        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
                        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);
                        
                        bool bNavProjectionSucceeded;
                        {
                             TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_NavMeshProjection_Call"));
                             bNavProjectionSucceeded = NavSys->ProjectPointToNavigation(NodeLocation, ProjectedNodeLocation, QueryExtent, DefaultNavData, NavQueryFilter);
                        }
                        if (bNavProjectionSucceeded)
                        {
                             BaseTargetLocation = ProjectedNodeLocation.Location;
                             bBaseTargetFound = true;
                        }
                    }
                }
            }
        }
    }

    if (!bBaseTargetFound) 
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_FallbackPath_MovementNodeProcessing"));
        TArray<USphereComponent*> MovementNodes = TargetInterface->GetMovementNodes();
        if (MovementNodes.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, MovementNodes.Num() - 1);
            USphereComponent* SelectedNode = MovementNodes.IsValidIndex(RandomIndex) ? MovementNodes[RandomIndex] : nullptr;

            if (SelectedNode)
            {
                FVector NodeLocation = SelectedNode->GetComponentLocation();
                FCollisionQueryParams CollisionParams;
                CollisionParams.AddIgnoredActor(OwnerPawn);
                CollisionParams.AddIgnoredActor(TargetActor);

                FVector StartTrace = NodeLocation;
                FVector EndTrace = NodeLocation - FVector(0.f, 0.f, GroundTraceDistance + SelectedNode->GetScaledSphereRadius());

                bool bGroundTraceSucceeded;
                {
                    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_LineTrace_GroundSearch"));
                    bGroundTraceSucceeded = World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, GroundTraceChannel, CollisionParams);
                }

                if (bGroundTraceSucceeded)
                {
                     if (NavSys && DefaultNavData) {
                        FVector QueryExtent(NavMeshProjectionHorizontalExtent * 0.25f, NavMeshProjectionHorizontalExtent * 0.25f, NavMeshProjectionVerticalThreshold);
                        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
                        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);
                        
                        bool bNavProjectionSucceeded;
                        {
                            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_NavMeshProjection_Call"));
                            bNavProjectionSucceeded = NavSys->ProjectPointToNavigation(HitResult.ImpactPoint, ProjectedImpactLocation, QueryExtent, DefaultNavData, NavQueryFilter);
                        }
                        if (bNavProjectionSucceeded) {
                            BaseTargetLocation = ProjectedImpactLocation.Location;
                            bBaseTargetFound = true;
                        }
                    } else { 
                        BaseTargetLocation = HitResult.ImpactPoint;
                        bBaseTargetFound = true;
                    }
                }
                else 
                {
                    if (NavSys && DefaultNavData) {
                        FVector QueryExtent(NavMeshProjectionHorizontalExtent * 0.5f, NavMeshProjectionHorizontalExtent * 0.5f, NavMeshProjectionVerticalThreshold);
                        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
                        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);
                        
                        bool bNavProjectionSucceeded;
                        {
                            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_NavMeshProjection_Call"));
                            bNavProjectionSucceeded = NavSys->ProjectPointToNavigation(NodeLocation, ProjectedNodeLoc, QueryExtent, DefaultNavData, NavQueryFilter);
                        }
                        if (bNavProjectionSucceeded) {
                            BaseTargetLocation = ProjectedNodeLoc.Location;
                            bBaseTargetFound = true;
                        }
                    } else { 
                        BaseTargetLocation = NodeLocation;
                        bBaseTargetFound = true;
                    }
                }
            }
        }
    }
    
    if (!bBaseTargetFound && NavSys && DefaultNavData)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_FinalFallback_TargetActorNavProjection"));
        FNavLocation ProjectedNavLocation;
        FVector QueryExtent(NavMeshProjectionHorizontalExtent, NavMeshProjectionHorizontalExtent, NavMeshProjectionVerticalThreshold);
        FVector TargetActorLocation = TargetActor->GetActorLocation();

        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);

        bool bNavProjectionSucceeded;
        {
            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_NavMeshProjection_Call"));
            bNavProjectionSucceeded = NavSys->ProjectPointToNavigation(TargetActorLocation,
                                                ProjectedNavLocation,
                                                QueryExtent,
                                                DefaultNavData,
                                                NavQueryFilter);
        }
        if (bNavProjectionSucceeded)
        {
            BaseTargetLocation = ProjectedNavLocation.Location;
            bBaseTargetFound = true;
        }
    }

    if (!bBaseTargetFound)
    {
        BlackboardComp->ClearValue(GetSelectedBlackboardKey());
        if (bDrawDebugTraceForDuration)
        {
            UE_LOG(LogTemp, Warning, TEXT("UBTService_TargetLocation: Could not find any valid target location."));
        }
        return;
    }
    
    FVector FinalTargetLocation = BaseTargetLocation;

    if (bApplyRandomOffset && NavSys && DefaultNavData)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_RandomOffsetLogic"));
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f;
        if (RandomDirection.IsNearlyZero()) RandomDirection = FVector(1.0f,0.0f,0.0f);
        RandomDirection.Normalize();

        float RandomDistance = FMath::FRandRange(0.0f, MaxRandomOffsetRadius);
        FVector OffsetAttemptLocation = BaseTargetLocation + RandomDirection * RandomDistance;

        FNavLocation ProjectedOffsetNavLocation;
        FVector OffsetProjectionExtent(MaxRandomOffsetRadius, MaxRandomOffsetRadius, NavMeshProjectionVerticalThreshold); 
        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
        FSharedConstNavQueryFilter OffsetNavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);

        bool bNavProjectionSucceeded;
        {
            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_TLG_NavMeshProjection_Call"));
            bNavProjectionSucceeded = NavSys->ProjectPointToNavigation(OffsetAttemptLocation,
                                                ProjectedOffsetNavLocation,
                                                OffsetProjectionExtent,
                                                DefaultNavData,
                                                OffsetNavQueryFilter);
        }
        if (bNavProjectionSucceeded)
        {
            FinalTargetLocation = ProjectedOffsetNavLocation.Location;
        }
    }
    
    if (bDrawDebugTraceForDuration)
    {
        DrawDebugSphere(World, BaseTargetLocation, 20.f, 12, FColor::Yellow, false, DebugTraceDuration, 0, 3.f);
        if (FinalTargetLocation != BaseTargetLocation)
        {
            DrawDebugLine(World, BaseTargetLocation, FinalTargetLocation, FColor::Orange, false, DebugTraceDuration, 0, 2.f);
            DrawDebugSphere(World, FinalTargetLocation, 30.f, 12, FColor::Magenta, false, DebugTraceDuration, 0, 5.f);
        }
        else
        {
             DrawDebugSphere(World, FinalTargetLocation, 30.f, 12, FColor::Blue, false, DebugTraceDuration, 0, 5.f);
        }
    }

    BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), FinalTargetLocation);
}