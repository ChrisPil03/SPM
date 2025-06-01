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

    bPreferNavMeshProjectionForAirborneTarget = true; // This flag now controls whether to use movement node trace for airborne
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
    
    // This is the original block that checks if the target is airborne/off-mesh
    if (bPreferNavMeshProjectionForAirborneTarget && NavSys && DefaultNavData)
    {
        ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
        bool bTargetIsLikelyOffNavMesh = true; // Default assumption
        if (TargetCharacter && TargetCharacter->GetCharacterMovement() &&
            TargetCharacter->GetCharacterMovement()->IsMovingOnGround() &&
            !TargetCharacter->GetCharacterMovement()->IsFalling())
        {
            bTargetIsLikelyOffNavMesh = false; // Target seems to be on ground and not falling
        }

        if (bTargetIsLikelyOffNavMesh) // If target is airborne or off-mesh
        {
            // --- MODIFIED: Use Movement Node + Downward Trace logic here ---
            TArray<USphereComponent*> MovementNodes = TargetInterface->GetMovementNodes();
            if (MovementNodes.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, MovementNodes.Num() - 1);
                USphereComponent* SelectedNode = MovementNodes.IsValidIndex(RandomIndex) ? MovementNodes[RandomIndex] : nullptr;

                if (SelectedNode)
                {
                    FVector NodeLocation = SelectedNode->GetComponentLocation();
                    FHitResult HitResult;
                    FCollisionQueryParams CollisionParams;
                    CollisionParams.AddIgnoredActor(OwnerPawn);
                    CollisionParams.AddIgnoredActor(TargetActor);

                    FVector StartTrace = NodeLocation;
                    FVector EndTrace = NodeLocation - FVector(0.f, 0.f, GroundTraceDistance + SelectedNode->GetScaledSphereRadius());

                    if (bDrawDebugTraceForDuration)
                    {
                        DrawDebugLine(World, StartTrace, EndTrace, FColor::Green, false, DebugTraceDuration, 0, 1.f);
                    }

                    if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, GroundTraceChannel, CollisionParams))
                    {
                        // We found a point on the ground below the movement node.
                        // Now, project THIS point to the NavMesh to ensure it's navigable.
                        FNavLocation ProjectedGroundPoint;
                        FVector QueryExtent(NavMeshProjectionHorizontalExtent * 0.25f, NavMeshProjectionHorizontalExtent * 0.25f, NavMeshProjectionVerticalThreshold); // Smaller extent for precise projection
                        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
                        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);

                        if (NavSys->ProjectPointToNavigation(HitResult.ImpactPoint, ProjectedGroundPoint, QueryExtent, DefaultNavData, NavQueryFilter))
                        {
                            BaseTargetLocation = ProjectedGroundPoint.Location;
                            bBaseTargetFound = true;
                            if (bDrawDebugTraceForDuration)
                            {
                                DrawDebugSphere(World, HitResult.ImpactPoint, 10.f, 12, FColor::Red, false, DebugTraceDuration, 0, 1.5f); // Original hit
                                DrawDebugSphere(World, BaseTargetLocation, 15.f, 12, FColor::Cyan, false, DebugTraceDuration, 0, 2.f); // Projected hit
                            }
                        }
                        // If projection of hit point fails, bBaseTargetFound remains false
                    }
                    else
                    {
                        // If ground trace from node fails, try projecting the node's original location.
                        FNavLocation ProjectedNodeLocation;
                        FVector QueryExtent(NavMeshProjectionHorizontalExtent * 0.5f, NavMeshProjectionHorizontalExtent * 0.5f, NavMeshProjectionVerticalThreshold);
                        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
                        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);
                        if (NavSys->ProjectPointToNavigation(NodeLocation, ProjectedNodeLocation, QueryExtent, DefaultNavData, NavQueryFilter))
                        {
                             BaseTargetLocation = ProjectedNodeLocation.Location;
                             bBaseTargetFound = true;
                        }
                        // If this also fails, bBaseTargetFound remains false
                    }
                }
            }
            // If no movement nodes, or selected node invalid, or all attempts failed, bBaseTargetFound is still false.
            // It will then fall through to the next `if (!bBaseTargetFound)` block.
            // --- END OF MOVEMENT NODE LOGIC FOR AIRBORNE ---
        }
        // If bTargetIsLikelyOffNavMesh was false (target is on ground), this block is skipped,
        // and bBaseTargetFound is still false, so it will proceed to the logic below.
    }

    // This is the original fallback logic that uses movement nodes if the above didn't find anything OR
    // if bPreferNavMeshProjectionForAirborneTarget was false OR if target was on ground.
    if (!bBaseTargetFound) 
    {
        TArray<USphereComponent*> MovementNodes = TargetInterface->GetMovementNodes();
        if (MovementNodes.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, MovementNodes.Num() - 1);
            USphereComponent* SelectedNode = MovementNodes.IsValidIndex(RandomIndex) ? MovementNodes[RandomIndex] : nullptr;

            if (SelectedNode)
            {
                FVector NodeLocation = SelectedNode->GetComponentLocation();
                FHitResult HitResult;
                FCollisionQueryParams CollisionParams;
                CollisionParams.AddIgnoredActor(OwnerPawn);
                CollisionParams.AddIgnoredActor(TargetActor);

                FVector StartTrace = NodeLocation;
                FVector EndTrace = NodeLocation - FVector(0.f, 0.f, GroundTraceDistance + SelectedNode->GetScaledSphereRadius());

                if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, GroundTraceChannel, CollisionParams))
                {
                    // Project this impact point to ensure navigability
                     if (NavSys && DefaultNavData) {
                        FNavLocation ProjectedImpactLocation;
                        FVector QueryExtent(NavMeshProjectionHorizontalExtent * 0.25f, NavMeshProjectionHorizontalExtent * 0.25f, NavMeshProjectionVerticalThreshold);
                        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
                        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);
                        if (NavSys->ProjectPointToNavigation(HitResult.ImpactPoint, ProjectedImpactLocation, QueryExtent, DefaultNavData, NavQueryFilter)) {
                            BaseTargetLocation = ProjectedImpactLocation.Location;
                            bBaseTargetFound = true;
                        }
                    } else { // No navsys, use raw impact point
                        BaseTargetLocation = HitResult.ImpactPoint;
                        bBaseTargetFound = true;
                    }
                }
                else // Trace failed
                {
                    // Project the original node location
                    if (NavSys && DefaultNavData) {
                        FNavLocation ProjectedNodeLoc;
                        FVector QueryExtent(NavMeshProjectionHorizontalExtent * 0.5f, NavMeshProjectionHorizontalExtent * 0.5f, NavMeshProjectionVerticalThreshold);
                        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
                        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);
                        if (NavSys->ProjectPointToNavigation(NodeLocation, ProjectedNodeLoc, QueryExtent, DefaultNavData, NavQueryFilter)) {
                            BaseTargetLocation = ProjectedNodeLoc.Location;
                            bBaseTargetFound = true;
                        }
                    } else { // No navsys, use raw node location
                        BaseTargetLocation = NodeLocation;
                        bBaseTargetFound = true;
                    }
                }
            }
        }
    }
    
    // Final fallback if absolutely nothing was found yet (e.g., no movement nodes, target is on ground but previous logic didn't set BaseTargetLocation)
    // This is where it would project the TargetActor's location itself if it's considered on ground by the logic
    // or if bPreferNavMeshProjectionForAirborneTarget was false and the movement node logic above didn't run or failed.
    // To ensure we don't re-project if the airborne section already failed, we check `bBaseTargetFound` again.
    if (!bBaseTargetFound && NavSys && DefaultNavData)
    {
        // This will usually only run if the target was considered "on ground" initially,
        // and the movement node logic in the second `if(!bBaseTargetFound)` block also failed.
        // Or if bPreferNavMeshProjectionForAirborneTarget was false.
        FNavLocation ProjectedNavLocation;
        FVector QueryExtent(NavMeshProjectionHorizontalExtent, NavMeshProjectionHorizontalExtent, NavMeshProjectionVerticalThreshold);
        FVector TargetActorLocation = TargetActor->GetActorLocation();

        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
        FSharedConstNavQueryFilter NavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);

        if (NavSys->ProjectPointToNavigation(TargetActorLocation,
                                            ProjectedNavLocation,
                                            QueryExtent,
                                            DefaultNavData,
                                            NavQueryFilter))
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

    // Apply Random Offset (this will project the offset point to NavMesh if possible)
    if (bApplyRandomOffset && NavSys && DefaultNavData)
    {
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

        if (NavSys->ProjectPointToNavigation(OffsetAttemptLocation,
                                            ProjectedOffsetNavLocation,
                                            OffsetProjectionExtent,
                                            DefaultNavData,
                                            OffsetNavQueryFilter))
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