#include "BTService_TargetLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "AIController.h" // For AAIController
#include "EnemyAI.h"
#include "Attackable.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavFilters/NavigationQueryFilter.h"

UBTService_TargetLocation::UBTService_TargetLocation()
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

void UBTService_TargetLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
    
    if (bPreferNavMeshProjectionForAirborneTarget && NavSys && DefaultNavData)
    {
        ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
        bool bTargetIsLikelyOffNavMesh = true;
        if (TargetCharacter && TargetCharacter->GetCharacterMovement() &&
            TargetCharacter->GetCharacterMovement()->IsMovingOnGround() &&
            !TargetCharacter->GetCharacterMovement()->IsFalling())
        {
            bTargetIsLikelyOffNavMesh = false;
        }

        if (bTargetIsLikelyOffNavMesh)
        {
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
    }

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
                    BaseTargetLocation = HitResult.ImpactPoint;
                    bBaseTargetFound = true;
                }
                else
                {
                    BaseTargetLocation = NodeLocation;
                    bBaseTargetFound = true;
                }
            }
        }
    }

    if (!bBaseTargetFound)
    {
        BlackboardComp->ClearValue(GetSelectedBlackboardKey());
        return;
    }
    
    FVector FinalTargetLocation = BaseTargetLocation;

    if (bApplyRandomOffset && NavSys && DefaultNavData)
    {
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f;
        if (RandomDirection.IsNearlyZero()) RandomDirection = FVector(1.0f,0.0f,0.0f);
        RandomDirection.Normalize();

        float RandomDistance = FMath::FRandRange(0.0f, MaxRandomOffsetRadius);
        FVector OffsetLocation = BaseTargetLocation + RandomDirection * RandomDistance;

        FNavLocation ProjectedOffsetLocation;
        FVector OffsetProjectionExtent(MaxRandomOffsetRadius * 0.5f, MaxRandomOffsetRadius * 0.5f, NavMeshProjectionVerticalThreshold);

        TSubclassOf<UNavigationQueryFilter> FilterClass = OwnerController->GetDefaultNavigationFilterClass();
        FSharedConstNavQueryFilter OffsetNavQueryFilter = UNavigationQueryFilter::GetQueryFilter(*DefaultNavData, OwnerController, FilterClass);

        if (NavSys->ProjectPointToNavigation(OffsetLocation,
                                            ProjectedOffsetLocation,
                                            OffsetProjectionExtent,
                                            DefaultNavData,
                                            OffsetNavQueryFilter))
        {
            FinalTargetLocation = ProjectedOffsetLocation.Location;
        }

    }
    
    if (bDrawDebugTraceForDuration)
    {
        if (bBaseTargetFound && FinalTargetLocation != BaseTargetLocation)
        {
            DrawDebugSphere(World, BaseTargetLocation, 20.f, 12, FColor::Yellow, false, DebugTraceDuration, 0, 3.f);
            DrawDebugLine(World, BaseTargetLocation, FinalTargetLocation, FColor::Orange, false, DebugTraceDuration, 0, 2.f);
            DrawDebugSphere(World, FinalTargetLocation, 30.f, 12, FColor::Magenta, false, DebugTraceDuration, 0, 5.f);
        }
        else if (bBaseTargetFound)
        {
             DrawDebugSphere(World, FinalTargetLocation, 30.f, 12, FColor::Blue, false, DebugTraceDuration, 0, 5.f);
        }
    }

    BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), FinalTargetLocation);
}