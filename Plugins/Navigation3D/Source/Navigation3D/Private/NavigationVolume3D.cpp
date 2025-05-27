// Fill out your copyright notice in the Description page of Project Settings.

#include "NavigationVolume3D.h" // Should be first for Precompiled Headers if used
#include "NavNode.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Reverse.h"
#include "Async/Async.h" // Required for Async task system
#include "GameFramework/Actor.h"


// Standard library includes
#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"


ANavigationVolume3D::ANavigationVolume3D()
{
    // ... (Constructor as before) ...
    PrimaryActorTick.bCanEverTick = false;

    DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>("DefaultSceneComponent");
    SetRootComponent(DefaultSceneComponent);

    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
    ProceduralMesh->SetupAttachment(GetRootComponent());
    ProceduralMesh->CastShadow = false;
    ProceduralMesh->SetEnableGravity(false);
    ProceduralMesh->bApplyImpulseOnDamage = false;
    ProceduralMesh->SetGenerateOverlapEvents(false);
    ProceduralMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
    ProceduralMesh->SetCollisionProfileName(FName("NoCollision"));
    ProceduralMesh->bHiddenInGame = true; // By default, for grid lines

    ObstacleObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    ObstacleObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
}

// ... (OnConstruction, BeginPlay, EndPlay, Tick, FindRandomValidLocationInRadius as before) ...
// Make sure FindRandomValidLocationInRadius does not rely on member debug arrays
// and uses GetWorld() safely if called from another thread (it's const, so `this` is const access)

bool ANavigationVolume3D::FindRandomValidLocationInRadius(
    const FVector& Origin,
    float WorldRadius,
    FVector& OutValidLocation,
    const AActor* ActorToIgnoreForLOS, // This AActor* needs to be valid if used
    int32 MaxCandidatesToCollect
) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindRandomValidLocationInRadius"));

    if (!bNodesInitializedAndFinalized || Nodes.IsEmpty() || WorldRadius < 0.0f || DivisionSize < KINDA_SMALL_NUMBER)
    {
        // ... (rest of the early out logic) ...
        if (WorldRadius < KINDA_SMALL_NUMBER && bNodesInitializedAndFinalized && !Nodes.IsEmpty() && DivisionSize > KINDA_SMALL_NUMBER)
        {
            const FIntVector OriginCoords = ConvertLocationToCoordinates(Origin);
            const NavNode* OriginNode = GetConstNode(OriginCoords);
            if (OriginNode && OriginNode->bIsTraversable)
            {
                OutValidLocation = ConvertCoordinatesToLocation(OriginNode->Coordinates);
                FHitResult HitResult;
                FCollisionQueryParams CollisionParams;
                // Ensure ActorToIgnoreForLOS is valid before adding
                if (ActorToIgnoreForLOS && IsValid(ActorToIgnoreForLOS)) { CollisionParams.AddIgnoredActor(ActorToIgnoreForLOS); }
                
                UWorld* World = GetWorld(); // GetWorld() on const this is fine
                bool bHasLineOfSight = true;
                if (World) { // World check is important
                    bHasLineOfSight = !World->LineTraceSingleByObjectType(
                        HitResult, Origin, OutValidLocation, FCollisionObjectQueryParams(ObstacleObjectTypes), CollisionParams);
                }
                if (bHasLineOfSight) return true;
            }
        }
        return false;
    }

    const FIntVector OriginGridCoords = ConvertLocationToCoordinates(Origin);
    const int32 BaseSearchRadiusInCells = FMath::CeilToInt(WorldRadius / DivisionSize);
    const int32 SearchExpansionCells = 1; 
    const int32 ExpandedSearchRadiusInCells = BaseSearchRadiusInCells + SearchExpansionCells;
    const float WorldRadiusSquared = FMath::Square(WorldRadius);

    UWorld* World = GetWorld(); // Fine for const method
    FCollisionQueryParams LOS_CollisionParams;
    // Ensure ActorToIgnoreForLOS is valid before adding
    if (ActorToIgnoreForLOS && IsValid(ActorToIgnoreForLOS)) { LOS_CollisionParams.AddIgnoredActor(ActorToIgnoreForLOS); }
    FCollisionObjectQueryParams LOS_ObjectQueryParams(ObstacleObjectTypes);

    for (int32 dz = ExpandedSearchRadiusInCells; dz >= -ExpandedSearchRadiusInCells; --dz) {
        for (int32 dy = -ExpandedSearchRadiusInCells; dy <= ExpandedSearchRadiusInCells; ++dy) {
            for (int32 dx = -ExpandedSearchRadiusInCells; dx <= ExpandedSearchRadiusInCells; ++dx) {
                const FIntVector CurrentGridCoords = OriginGridCoords + FIntVector(dx, dy, dz);
                if (!AreCoordinatesValid(CurrentGridCoords)) continue;

                const NavNode* CandidateNode = GetConstNode(CurrentGridCoords);
                if (CandidateNode && CandidateNode->bIsTraversable) {
                    const FVector NodeWorldCenter = ConvertCoordinatesToLocation(CandidateNode->Coordinates);
                    if (FVector::DistSquared(Origin, NodeWorldCenter) <= WorldRadiusSquared) {
                        bool bHasLineOfSight = true;
                        if (World) { // World check
                            FHitResult HitResult;
                            bHasLineOfSight = !World->LineTraceSingleByObjectType(
                                HitResult, Origin, NodeWorldCenter, LOS_ObjectQueryParams, LOS_CollisionParams);
                        }
                        if (bHasLineOfSight) {
                            OutValidLocation = NodeWorldCenter;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}


void ANavigationVolume3D::FindPathAsync(
    const AActor* RequestingActor,
    const FVector& StartLocation,
    const FVector& DestinationLocation,
    FOnPathfindingComplete OnCompleteCallback)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPathAsync_Launch"));

    // Use a TWeakObjectPtr to 'this' to ensure safety if the actor is destroyed
    TWeakObjectPtr<ANavigationVolume3D> WeakThis(this);
    TWeakObjectPtr<const AActor> WeakRequestingActor(RequestingActor);
    FString ActorName = RequestingActor ? RequestingActor->GetName() : TEXT("UnknownActor");

    // Increment search ID atomically. Relaxed memory order is fine as it's just for uniqueness.
    // Note: If this becomes 0, it will wrap around. The check for this inside ExecutePathfindingOnThread handles reset.
    uint32_t NewSearchID = AtomicPathfindingSearchIDCounter.fetch_add(1, std::memory_order_relaxed) + 1;
    if (NewSearchID == 0) { // Handle wrap-around, ensure it's never 0 if 0 is special
        NewSearchID = AtomicPathfindingSearchIDCounter.fetch_add(1, std::memory_order_relaxed) + 1;
         // This effectively means we might skip one ID on wraparound if multiple threads hit this.
         // But 0 is often a "not searched" marker.
    }


    Async(EAsyncExecution::Thread, [WeakThis, WeakRequestingActor, StartLocation, DestinationLocation, NewSearchID, OnCompleteCallback, ActorName]() {
        // This lambda runs on a background thread
        ANavigationVolume3D* StrongThis = WeakThis.Get();
        if (!StrongThis)
        {
            // 'this' actor was destroyed before or during async execution.
            // Schedule callback on game thread with an error.
            AsyncTask(ENamedThreads::GameThread, [OnCompleteCallback]() {
                if (OnCompleteCallback.IsBound())
                {
                    OnCompleteCallback.ExecuteIfBound(ENavigationVolumeResult::ENVR_VolumeNotReady, TArray<FVector>());
                }
            });
            return;
        }

        // Execute the core pathfinding logic
        FPathfindingInternalResultBundle ResultBundle = StrongThis->ExecutePathfindingOnThread(
            WeakThis, WeakRequestingActor, StartLocation, DestinationLocation, NewSearchID
        );

        // Pathfinding is done, schedule the callback and debug drawing on the game thread
        AsyncTask(ENamedThreads::GameThread, [WeakThis, ResultBundle, OnCompleteCallback]() {
            ANavigationVolume3D* GameThreadStrongThis = WeakThis.Get();
            if (!GameThreadStrongThis)
            {
                // Actor destroyed by the time we got back to game thread.
                // Still try to call callback if it's static or bound to something else.
                if (OnCompleteCallback.IsBound())
                {
                    OnCompleteCallback.ExecuteIfBound(ENavigationVolumeResult::ENVR_VolumeNotReady, ResultBundle.PathPoints);
                }
                return;
            }

            // Execute the callback provided by the caller
            if (OnCompleteCallback.IsBound())
            {
                OnCompleteCallback.ExecuteIfBound(ResultBundle.ResultCode, ResultBundle.PathPoints);
            }

            // Handle debug drawing
            if (ResultBundle.bShouldDrawThisPathAndExploration_TaskLocal)
            {
                UWorld* World = GameThreadStrongThis->GetWorld();
                if (World)
                {
                    GameThreadStrongThis->FlushCollectedDebugDraws(
                        World, 
                        GameThreadStrongThis->DebugDrawLifetime * (ResultBundle.bIsLongPath_TaskLocal ? 2.0f : 1.0f), 
                        ResultBundle.DebugSpheresToDraw_TaskLocal, 
                        ResultBundle.DebugLinesToDraw_TaskLocal,
                        ResultBundle.bIsLongPath_TaskLocal
                    );
                }
            }
            
            // Handle pausing for long paths
            if (ResultBundle.bIsLongPath_TaskLocal && GameThreadStrongThis->bDrawPathfindingDebug && GameThreadStrongThis->bPauseOnLongPath)
            {
                 UWorld* World = GameThreadStrongThis->GetWorld();
                 if (World) {
                    UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D (%s): Game PAUSED due to long path (%d steps). Actor: %s"), 
                        *GameThreadStrongThis->GetName(), ResultBundle.PathPoints.Num(), *ResultBundle.ActorNameForLog);
                    UGameplayStatics::SetGamePaused(World, true);
                 }
            }
        });
    });
}


ANavigationVolume3D::FPathfindingInternalResultBundle ANavigationVolume3D::ExecutePathfindingOnThread(
    TWeakObjectPtr<ANavigationVolume3D> WeakThisForConstAccess, // For accessing const members/methods if needed
    TWeakObjectPtr<const AActor> WeakRequestingActor,
    const FVector& StartLocation,
    const FVector& DestinationLocation,
    uint32_t PathSearchID)
{
    // This function runs on a background thread.
    // `this` pointer is valid here because it's called from a lambda where `StrongThis` was checked.
    // However, for const member access (like DivisionSize etc.), we might prefer the WeakThisForConstAccess if `this` actor could change
    // state in unexpected ways by other game thread operations. For this setup, direct `this->` is generally fine.

    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_Total"));
    
    const AActor* RequestingActorPtr = WeakRequestingActor.Get(); // Try to get strong ptr
    FString ActorNameForLogging = RequestingActorPtr ? RequestingActorPtr->GetName() : TEXT("UnknownOrInvalidActor");
    FPathfindingInternalResultBundle ResultBundle(ActorNameForLogging);


    if (!bNodesInitializedAndFinalized || Nodes.IsEmpty()) {
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - Nodes not initialized or empty. Actor: %s"), *ActorNameForLogging);
        ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_VolumeNotReady;
        return ResultBundle;
    }

    const uint32_t MySearchID = PathSearchID; // Use the passed-in, unique search ID

    // Special handling if MySearchID wraps to 0, assuming 0 is reserved for "unvisited in any search"
    // This is a safeguard; the ID generation in FindPathAsync tries to avoid giving 0.
    if (MySearchID == 0) {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - SearchID is 0. This is unexpected. Resetting all node SearchIDs. Actor: %s"), *ActorNameForLogging);
        for (NavNode& Node : Nodes) { // 'Nodes' is accessed via 'this'
            Node.SearchID_Pathfinding = 0; // Reset all to ensure clean state
            // Also reset A* data, as SearchID 0 might mean "never touched"
            Node.FScore_Pathfinding = std::numeric_limits<float>::max();
            Node.GScore_Pathfinding = std::numeric_limits<float>::max();
            Node.CameFrom_Pathfinding = nullptr;
        }
        // It's problematic to continue with SearchID 0 if 0 has special meaning.
        // For robustness, we could assign a fallback or error out.
        // Given FindPathAsync tries to avoid 0, this should be rare.
        // If it happens, it implies AtomicPathfindingSearchIDCounter wrapped and was incremented to 0, then 1.
        // A simple fix here is to just use a non-zero value or log an error and return.
        // For now, we log and proceed; the A* logic might behave unexpectedly if nodes naturally have SearchID 0.
        // A robust system would ensure MySearchID is never the "unvisited" marker.
        // Let's assume MySearchID will practically be > 0.
    }


    NavNode* StartNodePtr = nullptr;
    NavNode* EndNodePtr   = nullptr;

    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_NodeConversionAndValidation"));
        StartNodePtr = GetNode(ConvertLocationToCoordinates(StartLocation)); // GetNode uses 'this'
        EndNodePtr   = GetNode(ConvertLocationToCoordinates(DestinationLocation));

        if (!StartNodePtr) { 
            UE_LOG(LogTemp, Warning, TEXT("ExecutePathfindingOnThread: StartNodePtr is null. Actor: %s, StartLoc: %s"), *ActorNameForLogging, *StartLocation.ToString());
            ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_StartNodeInvalid; return ResultBundle;
        }
        if (!EndNodePtr) {
            UE_LOG(LogTemp, Warning, TEXT("ExecutePathfindingOnThread: EndNodePtr is null. Actor: %s, DestLoc: %s"), *ActorNameForLogging, *DestinationLocation.ToString());
            ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_EndNodeInvalid; return ResultBundle;
        }

        auto ResolveBlockedNode = [&](NavNode*& NodeToResolve, const FVector& OriginalWorldLocation, bool bIsStartNode) -> ENavigationVolumeResult {
            // Initialize node for this search if it hasn't been touched by this SearchID yet
            if (NodeToResolve->SearchID_Pathfinding != MySearchID) {
                NodeToResolve->SearchID_Pathfinding = MySearchID;
                NodeToResolve->GScore_Pathfinding = std::numeric_limits<float>::max();
                NodeToResolve->FScore_Pathfinding = std::numeric_limits<float>::max();
                NodeToResolve->CameFrom_Pathfinding = nullptr;
            }

            if (!NodeToResolve->bIsTraversable) {
                TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(bIsStartNode ? TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_ResolveBlockedStart") : TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_ResolveBlockedEnd"));
                float SearchRadius = 300.f; FVector FoundLocation;
                // FindRandomValidLocationInRadius is const, can be called via 'this'
                // Pass the resolved RequestingActorPtr for LOS checks
                if (this->FindRandomValidLocationInRadius(OriginalWorldLocation, SearchRadius, FoundLocation, RequestingActorPtr)) {
                    NodeToResolve = GetNode(ConvertLocationToCoordinates(FoundLocation));
                    if (!NodeToResolve) { 
                        UE_LOG(LogTemp, Warning, TEXT("ExecutePathfindingOnThread: ResolveBlockedNode found location but GetNode returned null. Actor: %s"), *ActorNameForLogging);
                        return bIsStartNode ? ENavigationVolumeResult::ENVR_StartNodeBlocked : ENavigationVolumeResult::ENVR_EndNodeBlocked;
                    }
                    // Initialize the new node for this search
                    NodeToResolve->SearchID_Pathfinding = MySearchID;
                    NodeToResolve->GScore_Pathfinding = std::numeric_limits<float>::max();
                    NodeToResolve->FScore_Pathfinding = std::numeric_limits<float>::max();
                    NodeToResolve->CameFrom_Pathfinding = nullptr;

                    if (!NodeToResolve->bIsTraversable) return bIsStartNode ? ENavigationVolumeResult::ENVR_StartNodeBlocked : ENavigationVolumeResult::ENVR_EndNodeBlocked;
                } else { return bIsStartNode ? ENavigationVolumeResult::ENVR_StartNodeBlocked : ENavigationVolumeResult::ENVR_EndNodeBlocked; }
            }
            return ENavigationVolumeResult::ENVR_Success;
        };

        ENavigationVolumeResult StartResolveResult = ResolveBlockedNode(StartNodePtr, StartLocation, true);
        if(StartResolveResult != ENavigationVolumeResult::ENVR_Success) { ResultBundle.ResultCode = StartResolveResult; return ResultBundle; }
        
        ENavigationVolumeResult EndResolveResult = ResolveBlockedNode(EndNodePtr, DestinationLocation, false);
        if(EndResolveResult != ENavigationVolumeResult::ENVR_Success) { ResultBundle.ResultCode = EndResolveResult; return ResultBundle; }
        
        if (bDrawPathfindingDebug) { // Check general debug flag
            AddDebugSphere_TaskLocal(ResultBundle.DebugSpheresToDraw_TaskLocal, ConvertCoordinatesToLocation(StartNodePtr->Coordinates), DebugNodeSphereRadius * 1.5f, FColor::Cyan, 12);
            AddDebugSphere_TaskLocal(ResultBundle.DebugSpheresToDraw_TaskLocal, ConvertCoordinatesToLocation(EndNodePtr->Coordinates), DebugNodeSphereRadius * 1.5f, FColor::Magenta, 12);
        }
        
        if (StartNodePtr == EndNodePtr) { 
            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_PathToSelf"));
            ResultBundle.PathPoints.Add(ConvertCoordinatesToLocation(StartNodePtr->Coordinates));
            UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - Path to self found. Actor: %s. Path steps: %d"), *ActorNameForLogging, ResultBundle.PathPoints.Num());
            
            ResultBundle.bIsLongPath_TaskLocal = ResultBundle.PathPoints.Num() > LongPathThreshold;
            ResultBundle.bShouldDrawThisPathAndExploration_TaskLocal = bDrawPathfindingDebug && (ResultBundle.bIsLongPath_TaskLocal || !bOnlyDrawDebugForLongPaths);
            
            ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_PathToSelf;
            return ResultBundle;
        }
    }

    std::priority_queue<NavNode*, std::vector<NavNode*>, NodeCompare> OpenSet;
    // ClosedSet needs to be per-search-instance if multiple searches can run truly concurrently on same nodes.
    // The SearchID mechanism tries to avoid full resets, but concurrent writes to G/F/CameFrom are an issue.
    // For this refactor, we assume SearchID is sufficient for typical usage patterns (not massively parallel searches in same area).
    // If extremely high concurrency on the same nodes is needed, NavNode's A* data would need to be externalized per search.
    std::unordered_set<NavNode*> ClosedSetForThisSearchInstance; // Local to this pathfinding call

    auto HeuristicCost = [EndNodePtr](const NavNode* Node) -> float { if (!Node || !EndNodePtr) return std::numeric_limits<float>::max(); return FVector::Distance(FVector(EndNodePtr->Coordinates), FVector(Node->Coordinates)); };
    auto NeighborDistance = [](const NavNode* Node1, const NavNode* Node2) -> float { 
        if (!Node1 || !Node2) return std::numeric_limits<float>::max();
        float dx = static_cast<float>(FMath::Abs(Node1->Coordinates.X - Node2->Coordinates.X));
        float dy = static_cast<float>(FMath::Abs(Node1->Coordinates.Y - Node2->Coordinates.Y));
        float dz = static_cast<float>(FMath::Abs(Node1->Coordinates.Z - Node2->Coordinates.Z));
        return FMath::Sqrt(dx*dx + dy*dy + dz*dz); // Using actual distance
    };

    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_AStar_Init"));
        // Ensure StartNodePtr is initialized for MySearchID (done in ResolveBlockedNode)
        ensureMsgf(StartNodePtr->SearchID_Pathfinding == MySearchID, TEXT("StartNodePtr not initialized for current SearchID before A* Init! Actor: %s"), *ActorNameForLogging);
        StartNodePtr->GScore_Pathfinding = 0.0f;
        StartNodePtr->FScore_Pathfinding = HeuristicCost(StartNodePtr);
        OpenSet.push(StartNodePtr);
        if (bDrawPathfindingDebug) {
            AddDebugSphere_TaskLocal(ResultBundle.DebugSpheresToDraw_TaskLocal, ConvertCoordinatesToLocation(StartNodePtr->Coordinates), DebugNodeSphereRadius, FColor::Green);
        }
    }

    bool bPathSuccessfullyFound = false;
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_AStar_MainLoop"));
        while (!OpenSet.empty())
        {
            NavNode* Current = OpenSet.top(); OpenSet.pop();

            // Node might have been processed if pushed multiple times with better paths, or if SearchID is old.
            // Crucially, ensure it's part of *this specific* search via MySearchID.
            if (!Current || Current->SearchID_Pathfinding != MySearchID) {
                 continue;
            }
            
            if (bDrawPathfindingDebug) {
                 AddDebugSphere_TaskLocal(ResultBundle.DebugSpheresToDraw_TaskLocal, ConvertCoordinatesToLocation(Current->Coordinates), DebugNodeSphereRadius * 1.2f, FColor::Yellow);
            }
            
            // Check if Current is already in the ClosedSet for *this search instance*.
            if (ClosedSetForThisSearchInstance.count(Current)) {
                continue;
            }
            ClosedSetForThisSearchInstance.insert(Current);

            if (bDrawPathfindingDebug) {
                AddDebugSphere_TaskLocal(ResultBundle.DebugSpheresToDraw_TaskLocal, ConvertCoordinatesToLocation(Current->Coordinates), DebugNodeSphereRadius, FColor::Red);
            }
            
            if (Current == EndNodePtr) {
                bPathSuccessfullyFound = true;
                break; 
            }

            {
                TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_NeighborLoop"));
                // If GScore is max, this path to Current is not viable. (Should be caught by SearchID check or ClosedSet mostly)
                if (Current->GScore_Pathfinding == std::numeric_limits<float>::max()) { continue; }

                for (NavNode* Neighbor : Current->Neighbors) {
                    if (!Neighbor || !Neighbor->bIsTraversable) continue;

                    if (bDrawPathfindingDebug) {
                        AddDebugLine_TaskLocal(ResultBundle.DebugLinesToDraw_TaskLocal, ConvertCoordinatesToLocation(Current->Coordinates), ConvertCoordinatesToLocation(Neighbor->Coordinates), FColor(128,128,128,100), 0.5f);
                    }
                    
                    // Check if Neighbor is already in the ClosedSet for *this search instance*.
                    if (ClosedSetForThisSearchInstance.count(Neighbor)) {
                        continue;
                    }

                    float DistCurrentToNeighbor = NeighborDistance(Current, Neighbor);
                    if (DistCurrentToNeighbor == std::numeric_limits<float>::max()) continue; // Should not happen with valid nodes
                    
                    float TentativeGScore = Current->GScore_Pathfinding + DistCurrentToNeighbor;

                    // If neighbor is from an old search or not yet visited in this one, initialize it.
                    if (Neighbor->SearchID_Pathfinding != MySearchID) {
                        Neighbor->SearchID_Pathfinding = MySearchID;
                        Neighbor->GScore_Pathfinding = std::numeric_limits<float>::max();
                        Neighbor->FScore_Pathfinding = std::numeric_limits<float>::max();
                        Neighbor->CameFrom_Pathfinding = nullptr;
                    }

                    if (TentativeGScore < Neighbor->GScore_Pathfinding) {
                        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_UpdateNeighborPath"));
                        Neighbor->CameFrom_Pathfinding = Current;
                        Neighbor->GScore_Pathfinding = TentativeGScore;
                        Neighbor->FScore_Pathfinding = TentativeGScore + HeuristicCost(Neighbor); 
                        OpenSet.push(Neighbor);
                        if (bDrawPathfindingDebug) {
                            AddDebugSphere_TaskLocal(ResultBundle.DebugSpheresToDraw_TaskLocal, ConvertCoordinatesToLocation(Neighbor->Coordinates), DebugNodeSphereRadius, FColor::Green);
                            AddDebugLine_TaskLocal(ResultBundle.DebugLinesToDraw_TaskLocal, ConvertCoordinatesToLocation(Current->Coordinates), ConvertCoordinatesToLocation(Neighbor->Coordinates), FColor::White);
                        }
                    }
                }
            }
        }
    }

    if (bPathSuccessfullyFound) {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_PathReconstruction"));
        TArray<FVector> TempPath;
        NavNode* PathNode = EndNodePtr;
        while (PathNode != nullptr && PathNode->SearchID_Pathfinding == MySearchID) { // Ensure part of current search
            TempPath.Add(ConvertCoordinatesToLocation(PathNode->Coordinates));
            PathNode = PathNode->CameFrom_Pathfinding;
        }
        Algo::Reverse(TempPath);
        ResultBundle.PathPoints = TempPath;
        UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - Path successfully found. Actor: %s. Steps: %d"), *ActorNameForLogging, ResultBundle.PathPoints.Num());

        ResultBundle.bIsLongPath_TaskLocal = ResultBundle.PathPoints.Num() > LongPathThreshold;
        ResultBundle.bShouldDrawThisPathAndExploration_TaskLocal = bDrawPathfindingDebug && (ResultBundle.bIsLongPath_TaskLocal || !bOnlyDrawDebugForLongPaths);

        if (bDrawPathfindingDebug && ResultBundle.bShouldDrawThisPathAndExploration_TaskLocal) { // Further check if these specific draws are needed
            NavNode* VisPathNode = EndNodePtr;
            while (VisPathNode != nullptr && VisPathNode->SearchID_Pathfinding == MySearchID) {
                FVector NodeLoc = ConvertCoordinatesToLocation(VisPathNode->Coordinates);
                AddDebugSphere_TaskLocal(ResultBundle.DebugSpheresToDraw_TaskLocal, NodeLoc, DebugNodeSphereRadius * 0.9f, FColorList::NeonPink, 10);
                if (VisPathNode->CameFrom_Pathfinding && VisPathNode->CameFrom_Pathfinding->SearchID_Pathfinding == MySearchID) {
                    AddDebugLine_TaskLocal(ResultBundle.DebugLinesToDraw_TaskLocal, NodeLoc, ConvertCoordinatesToLocation(VisPathNode->CameFrom_Pathfinding->Coordinates), FColorList::NeonPink, 3.5f);
                }
                VisPathNode = VisPathNode->CameFrom_Pathfinding;
            }
        }
        
        if (ResultBundle.bIsLongPath_TaskLocal && bDrawPathfindingDebug) { // For logging long paths specifically
            UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - LONG PATH DETECTED: Actor: %s, %d steps (Threshold: %d)"), *ActorNameForLogging, ResultBundle.PathPoints.Num(), LongPathThreshold);
            // Pausing logic is handled on game thread based on bIsLongPath_TaskLocal
        }
        ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_Success;
        return ResultBundle;
    }

    // Path not found
    UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - Failed to find path. Actor: %s, StartNode: %s, EndNode: %s, StartLoc: %s, DestLoc: %s"),
        *ActorNameForLogging,
        (StartNodePtr ? *StartNodePtr->Coordinates.ToString() : TEXT("INVALID_START")),
        (EndNodePtr ? *EndNodePtr->Coordinates.ToString() : TEXT("INVALID_END")),
        *StartLocation.ToString(),
        *DestinationLocation.ToString());
    
    ResultBundle.bShouldDrawThisPathAndExploration_TaskLocal = bDrawPathfindingDebug && !bOnlyDrawDebugForLongPaths; // Draw explored if path failed & not in "only long paths" mode.
    ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_NoPathExists;
    return ResultBundle;
}


void ANavigationVolume3D::AddDebugSphere_TaskLocal(TArray<FDebugSphereData>& DebugSpheresArray, const FVector& Center, float Radius, const FColor& InSphereColor, int32 Segments) const
{
    // This const method assumes bDrawPathfindingDebug is a member of ANavigationVolume3D.
    // It's called from ExecutePathfindingOnThread, which has a valid 'this'.
    if (!bDrawPathfindingDebug) return; 
    DebugSpheresArray.Add({Center, Radius, InSphereColor, Segments});
}

void ANavigationVolume3D::AddDebugLine_TaskLocal(TArray<FDebugLineData>& DebugLinesArray, const FVector& Start, const FVector& End, const FColor& InLineColor, float Thickness) const
{
    if (!bDrawPathfindingDebug) return;
    DebugLinesArray.Add({Start, End, InLineColor, Thickness});
}

void ANavigationVolume3D::FlushCollectedDebugDraws(UWorld* World, float Lifetime, const TArray<FDebugSphereData>& Spheres, const TArray<FDebugLineData>& Lines, bool bIsLongPathContext) const
{
    // This const method assumes bDrawPathfindingDebug is a member.
    // Called on Game Thread.
    if (!World || !bDrawPathfindingDebug) 
    {
        return;
    }

    float ActualLifetime = Lifetime;
    // Example of adjusting lifetime based on context, though FindPathAsync already calculates it.
    // if (bIsLongPathContext && bPauseOnLongPath) { ActualLifetime *= 2.0f; }


    for (const auto& SphereData : Spheres)
    {
        DrawDebugSphere(World, SphereData.Center, SphereData.Radius, SphereData.Segments, SphereData.Color, false, ActualLifetime, 0, SphereData.Radius / 10.f);
    }
    for (const auto& LineData : Lines)
    {
        DrawDebugLine(World, LineData.Start, LineData.End, LineData.Color, false, ActualLifetime, 0, LineData.Thickness);
    }
    // The passed-in arrays 'Spheres' and 'Lines' are local to the calling scope (the game thread lambda)
    // and will be cleaned up automatically. No need to Empty() them here.
}


// ... (GetNode, GetConstNode, ClampCoordinates, AreCoordinatesValid, PrecomputeNodeTraversability, CreateLine, OnConstruction, BeginPlay, EndPlay, Tick, ConvertLocationToCoordinates, ConvertCoordinatesToLocation as before)
// Ensure these helper functions are robust and thread-aware if necessary (most are const or operate on local data).
// GetNode and GetConstNode are fine as they access 'this->Nodes' which is initialized in BeginPlay and then mostly read.
// The A* data within NavNode is the main concurrent access concern, partially mitigated by SearchID.

// Example of how BeginPlay might look
void ANavigationVolume3D::BeginPlay()
{
    Super::BeginPlay();

    if (bNodesInitializedAndFinalized) {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D (%s): BeginPlay - Nodes already initialized. Skipping."), *GetName());
        return;
    }
    // ... (rest of BeginPlay as provided in the problem, it's generally fine) ...
    const int32 TotalNodes = GetTotalDivisions();
    if (TotalNodes <= 0) {
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D (%s): BeginPlay - Invalid dimensions (X=%d, Y=%d, Z=%d). Cannot initialize nodes."), *GetName(), DivisionsX, DivisionsY, DivisionsZ);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): BeginPlay - Initializing %d nodes."), *GetName(), TotalNodes);

    Nodes.Empty(TotalNodes); 
    Nodes.SetNum(TotalNodes); 
                              
    for (int32 z = 0; z < DivisionsZ; ++z) {
        for (int32 y = 0; y < DivisionsY; ++y) {
            for (int32 x = 0; x < DivisionsX; ++x) {
                FIntVector CurrentCoords(x, y, z);
                NavNode* CurrentNodePtr = GetNode(CurrentCoords); 
                if (CurrentNodePtr) {
                    CurrentNodePtr->Coordinates = CurrentCoords;
                    CurrentNodePtr->bIsTraversable = true; 
                    CurrentNodePtr->Neighbors.clear(); 
                    // A* specific data like SearchID_Pathfinding is default-initialized (e.g., to 0).
                    // No need to set SearchID_Pathfinding here, it's set per-search.
                } else {
                    UE_LOG(LogTemp, Fatal, TEXT("ANavigationVolume3D (%s): BeginPlay - GetNode returned nullptr for valid grid coordinate %s during Phase 1. This should not happen."), *GetName(), *CurrentCoords.ToString());
                    ensureMsgf(false, TEXT("GetNode returned null for an expected valid coordinate during node initialization."));
                    return;
                }
            }
        }
    }
    
    auto addNeighborIfValid = [&](NavNode* CenterNode, const FIntVector& NeighborGridCoords) {
        // ... (as before)
        if (!CenterNode) return;
        if (AreCoordinatesValid(NeighborGridCoords)) {
            int32 SharedAxes = 0;
            if (CenterNode->Coordinates.X == NeighborGridCoords.X) SharedAxes++;
            if (CenterNode->Coordinates.Y == NeighborGridCoords.Y) SharedAxes++;
            if (CenterNode->Coordinates.Z == NeighborGridCoords.Z) SharedAxes++;

            if (SharedAxes >= MinSharedNeighborAxes && SharedAxes < 3) { 
                NavNode* NeighborNodePtr = GetNode(NeighborGridCoords);
                if (NeighborNodePtr) {
                    CenterNode->Neighbors.push_back(NeighborNodePtr);
                } else {
                     UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D (%s): BeginPlay - addNeighborIfValid got NULL for valid-seeming neighbor coords %s for center node %s"),
                        *GetName(), *NeighborGridCoords.ToString(), *CenterNode->Coordinates.ToString());
                     ensureMsgf(false, TEXT("addNeighborIfValid got NULL neighbor pointer for presumably valid coordinates."));
                }
            }
        }
    };

    for (NavNode& CurrentNode : Nodes) { 
        for (int dz = -1; dz <= 1; ++dz) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0 && dz == 0) continue; 
                    addNeighborIfValid(&CurrentNode, CurrentNode.Coordinates + FIntVector(dx, dy, dz));
                }
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): BeginPlay - Node neighbor population finished."), *GetName());

    PrecomputeNodeTraversability();
    bNodesInitializedAndFinalized = true;
    AtomicPathfindingSearchIDCounter.store(0); // Initialize atomic counter
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): BeginPlay - Initialization complete and finalized."), *GetName());
}

// GetNode / GetConstNode (ensure they are correct)
NavNode* ANavigationVolume3D::GetNode(FIntVector Coordinates) {
    FIntVector ClampedCoords = Coordinates;
    ClampCoordinates(ClampedCoords); 

    if (DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0) {
        return nullptr;
    }
    if (Nodes.IsEmpty()) { 
        return nullptr;
    }

    const int32 Index = (ClampedCoords.Z * (DivisionsX * DivisionsY)) + (ClampedCoords.Y * DivisionsX) + ClampedCoords.X;

    if (!Nodes.IsValidIndex(Index)) {
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D::GetNode - Calculated Index %d is OUT OF BOUNDS for Nodes array (Size: %d). Original Coords: %s, Clamped: %s, Divisions: %d,%d,%d"),
               Index, Nodes.Num(), *Coordinates.ToString(), *ClampedCoords.ToString(), DivisionsX, DivisionsY, DivisionsZ);
        ensureMsgf(false, TEXT("GetNode: Calculated Index is out of bounds!"));
        return nullptr;
    }
    return &Nodes[Index];
}

const NavNode* ANavigationVolume3D::GetConstNode(FIntVector Coordinates) const {
    FIntVector ClampedCoords = Coordinates;
    ClampCoordinates(ClampedCoords);

    if (DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0) return nullptr;
    if (Nodes.IsEmpty()) return nullptr;
    
    const int32 Index = (ClampedCoords.Z * (DivisionsX * DivisionsY)) + (ClampedCoords.Y * DivisionsX) + ClampedCoords.X;

    if (!Nodes.IsValidIndex(Index)) {
         UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D::GetConstNode - Calculated Index %d is OUT OF BOUNDS for Nodes array (Size: %d). Original Coords: %s, Clamped: %s, Divisions: %d,%d,%d"),
               Index, Nodes.Num(), *Coordinates.ToString(), *ClampedCoords.ToString(), DivisionsX, DivisionsY, DivisionsZ);
        ensureMsgf(false, TEXT("GetConstNode: Calculated Index is out of bounds!"));
        return nullptr;
    }
    return &Nodes[Index];
}

void ANavigationVolume3D::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    DivisionsX = FMath::Max(1, DivisionsX);
    DivisionsY = FMath::Max(1, DivisionsY);
    DivisionsZ = FMath::Max(1, DivisionsZ);
    DivisionSize = FMath::Max(1.0f, DivisionSize);
    LineThickness = FMath::Max(0.1f, LineThickness);

    if (bDrawGridLinesInEditor)
    {
        TArray<FVector> Vertices;
        TArray<int32> Triangles;
        const float GridSizeX_Local = static_cast<float>(DivisionsX) * DivisionSize; // Use local calculation
        const float GridSizeY_Local = static_cast<float>(DivisionsY) * DivisionSize;
        const float GridSizeZ_Local = static_cast<float>(DivisionsZ) * DivisionSize;
        FVector Start, End;

        for (int32 z = 0; z <= DivisionsZ; ++z) {
            for (int32 x = 0; x <= DivisionsX; ++x) {
                Start.Set(x * DivisionSize, 0.0f, z * DivisionSize);
                End.Set(x * DivisionSize, GridSizeY_Local, z * DivisionSize);
                CreateLine(Start, End, FVector::RightVector, Vertices, Triangles);
            }
        }
        for (int32 z = 0; z <= DivisionsZ; ++z) {
            for (int32 y = 0; y <= DivisionsY; ++y) {
                Start.Set(0.0f, y * DivisionSize, z * DivisionSize);
                End.Set(GridSizeX_Local, y * DivisionSize, z * DivisionSize);
                CreateLine(Start, End, FVector::ForwardVector, Vertices, Triangles);
            }
        }
        for (int32 x = 0; x <= DivisionsX; ++x) {
            for (int32 y = 0; y <= DivisionsY; ++y) {
                Start.Set(x * DivisionSize, y * DivisionSize, 0.0f);
                End.Set(x * DivisionSize, y * DivisionSize, GridSizeZ_Local);
                CreateLine(Start, End, FVector::UpVector, Vertices, Triangles);
            }
        }

        TArray<FVector> Normals; TArray<FVector2D> UVs; TArray<FColor> Colors; TArray<FProcMeshTangent> Tangents;
        ProceduralMesh->ClearMeshSection(0);
        if(Vertices.Num() > 0 && Triangles.Num() > 0) {
            ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, false);
        }

        if (IsValid(GridMaterial)) {
            UMaterialInstanceDynamic* DynamicMaterialInstance = Cast<UMaterialInstanceDynamic>(ProceduralMesh->GetMaterial(0));
            if (!DynamicMaterialInstance || DynamicMaterialInstance->GetBaseMaterial() != GridMaterial) {
                 DynamicMaterialInstance = ProceduralMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, GridMaterial);
            }
            if (DynamicMaterialInstance) {
                DynamicMaterialInstance->SetVectorParameterValue(FName("Color"), Color);
            } else {
                 ProceduralMesh->SetMaterial(0, GridMaterial);
            }
        } else {
            ProceduralMesh->SetMaterial(0, nullptr);
        }
    }
    else {
        ProceduralMesh->ClearMeshSection(0);
        ProceduralMesh->SetMaterial(0, nullptr);
    }
}

void ANavigationVolume3D::ClampCoordinates(FIntVector& InOutCoordinates) const
{
    // Clamp only if divisions are valid, otherwise behavior is undefined for Divisions-1
    if (DivisionsX > 0) InOutCoordinates.X = FMath::Clamp(InOutCoordinates.X, 0, DivisionsX - 1);
    else InOutCoordinates.X = 0;

    if (DivisionsY > 0) InOutCoordinates.Y = FMath::Clamp(InOutCoordinates.Y, 0, DivisionsY - 1);
    else InOutCoordinates.Y = 0;

    if (DivisionsZ > 0) InOutCoordinates.Z = FMath::Clamp(InOutCoordinates.Z, 0, DivisionsZ - 1);
    else InOutCoordinates.Z = 0;
}

bool ANavigationVolume3D::AreCoordinatesValid(const FIntVector& Coordinates) const
{
    if (DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0) return false; // Grid is not valid
    return Coordinates.X >= 0 && Coordinates.X < DivisionsX &&
           Coordinates.Y >= 0 && Coordinates.Y < DivisionsY &&
           Coordinates.Z >= 0 && Coordinates.Z < DivisionsZ;
}


void ANavigationVolume3D::PrecomputeNodeTraversability()
{
     UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Precomputing node traversability..."), *GetName());
     int32 NonTraversableCount = 0;

     if(ObstacleObjectTypes.Num() == 0 && ObstacleActorClassFilter == nullptr)
     {
         UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D (%s): No ObstacleObjectTypes or ObstacleActorClassFilter specified. All nodes considered traversable by overlap."), *GetName());
         return; // All nodes remain bIsTraversable = true by default
     }

     const FVector HalfBoxExtent(DivisionSize * 0.45f); // Slightly smaller to avoid issues at exact boundaries
     TArray<AActor*> ActorsToIgnore; 
     TArray<AActor*> OutActors;

     for (NavNode& NodeToCheck : Nodes)
     {
         const FVector WorldLocation = ConvertCoordinatesToLocation(NodeToCheck.Coordinates);
         OutActors.Empty(); 

         bool bOverlapped = UKismetSystemLibrary::BoxOverlapActors(
             this, 
             WorldLocation,
             HalfBoxExtent,
             ObstacleObjectTypes,
             ObstacleActorClassFilter,
             ActorsToIgnore,
             OutActors
         );

         NodeToCheck.bIsTraversable = !bOverlapped;
         if (!NodeToCheck.bIsTraversable) {
             NonTraversableCount++;
         }
     }
     UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Traversability precomputation complete. Found %d non-traversable nodes."), *GetName(), NonTraversableCount);
}

void ANavigationVolume3D::CreateLine(const FVector& Start, const FVector& End, const FVector& Normal, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    // ... (Your CreateLine implementation seems generally okay, just ensure Normal usage is correct for quad orientation)
    const float HalfLineThickness = LineThickness * 0.5f;
    FVector LineDirection = End - Start;
    if (!LineDirection.Normalize()) return; 

    FVector ThicknessDir1 = FVector::CrossProduct(LineDirection, Normal).GetSafeNormal();
    if (ThicknessDir1.IsNearlyZero()) 
    {
         FVector NonParallelNormal = Normal.GetSafeNormal();
         if(FMath::Abs(FVector::DotProduct(LineDirection, NonParallelNormal)) > 0.999f) // If Normal is (anti)parallel
         {
            LineDirection.FindBestAxisVectors(ThicknessDir1, NonParallelNormal); // Find an arbitrary perpendicular
         }
         ThicknessDir1 = FVector::CrossProduct(LineDirection, NonParallelNormal).GetSafeNormal();
         // If still zero, means LineDirection was zero or something is wrong
         if (ThicknessDir1.IsNearlyZero()) ThicknessDir1 = FVector(0,1,0); // Fallback
    }
    FVector ThicknessDir2 = FVector::CrossProduct(LineDirection, ThicknessDir1).GetSafeNormal();


    auto CreateQuad = [&](const FVector& ThicknessDirection)
    {
        int32 VtxStartIdx = Vertices.Num();
        Vertices.Add(Start + ThicknessDirection * HalfLineThickness); 
        Vertices.Add(End   + ThicknessDirection * HalfLineThickness); 
        Vertices.Add(Start - ThicknessDirection * HalfLineThickness); 
        Vertices.Add(End   - ThicknessDirection * HalfLineThickness); 

        Triangles.Add(VtxStartIdx + 0); Triangles.Add(VtxStartIdx + 1); Triangles.Add(VtxStartIdx + 2);
        Triangles.Add(VtxStartIdx + 2); Triangles.Add(VtxStartIdx + 1); Triangles.Add(VtxStartIdx + 3);
    };

    CreateQuad(ThicknessDir1);
    CreateQuad(ThicknessDir2);
}

FIntVector ANavigationVolume3D::ConvertLocationToCoordinates(const FVector& Location) const
{
    // Ensure the volume is valid before proceeding
    if (DivisionSize < KINDA_SMALL_NUMBER || DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ConvertLocationToCoordinates - Invalid DivisionSize or Dimensions. Returning zero vector."));
        return FIntVector::ZeroValue;
    }

    FIntVector Coordinates;
    // Transform the world location to the local space of the NavigationVolume actor
    const FVector GridSpaceLocation = GetActorTransform().InverseTransformPosition(Location);

    // Calculate integer coordinates based on division size
    // Using FloorToInt for consistent mapping of locations to grid cells
    Coordinates.X = FMath::FloorToInt(GridSpaceLocation.X / DivisionSize);
    Coordinates.Y = FMath::FloorToInt(GridSpaceLocation.Y / DivisionSize);
    Coordinates.Z = FMath::FloorToInt(GridSpaceLocation.Z / DivisionSize);

    // It's generally better to NOT clamp here directly if the function's purpose is pure conversion.
    // Clamping should ideally happen when using these coordinates to access the grid (e.g., in GetNode).
    // However, if you intend for this function to always return coordinates within the defined grid,
    // then clamping here is acceptable, but be aware of its implications.
    // For now, let's keep the clamping as it's often expected for Blueprint-exposed functions.
    FIntVector ClampedCoordinates = Coordinates; // Make a copy for clamping if you want to return clamped
    ClampCoordinates(ClampedCoordinates); // ClampCoordinates modifies its parameter
    return ClampedCoordinates; // Or return 'Coordinates' if you don't want clamping in this specific function
}

FVector ANavigationVolume3D::ConvertCoordinatesToLocation(const FIntVector& Coordinates) const
{
    // Ensure the volume is valid before proceeding
    if (DivisionSize < KINDA_SMALL_NUMBER || DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ConvertCoordinatesToLocation - Invalid DivisionSize or Dimensions. Returning zero vector."));
        return FVector::ZeroVector;
    }
    
    FIntVector ClampedCoordinates = Coordinates;
    // Clamp the input coordinates to be within the valid grid range before calculating world location.
    // This ensures that even if slightly out-of-bounds grid coordinates are passed,
    // we attempt to return a sensible location at the edge of the volume.
    ClampCoordinates(ClampedCoordinates);

    FVector GridSpaceLocation;
    // Calculate location at the center of the grid cell
    GridSpaceLocation.X = (static_cast<float>(ClampedCoordinates.X) + 0.5f) * DivisionSize;
    GridSpaceLocation.Y = (static_cast<float>(ClampedCoordinates.Y) + 0.5f) * DivisionSize;
    GridSpaceLocation.Z = (static_cast<float>(ClampedCoordinates.Z) + 0.5f) * DivisionSize;

    // Transform the local grid space location back to world space
    return GetActorTransform().TransformPosition(GridSpaceLocation);
}

void ANavigationVolume3D::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): EndPlay called. Node count before empty: %d. Initialized: %s"), 
        *GetName(), Nodes.Num(), bNodesInitializedAndFinalized ? TEXT("true") : TEXT("false"));

    // It's good practice to clear std::vectors that hold raw pointers if there's any chance
    // the pointed-to objects are managed elsewhere or if other systems might try to access them
    // after this EndPlay but before full destruction. In this case, Nodes owns the NavNode objects.
    if (bNodesInitializedAndFinalized) // Only iterate if nodes were actually set up
    {
        for (NavNode& Node : Nodes)
        {
            Node.Neighbors.clear(); // Clear the pointers from the std::vector
        }
    }

    Nodes.Empty(); // Deallocates memory for NavNode objects if TArray owned them.
                   // For TArray<NavNode>, it calls destructors (which is default for NavNode)
                   // and deallocates.
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Nodes array emptied. Node count after empty: %d"), *GetName(), Nodes.Num());
    
    bNodesInitializedAndFinalized = false;
    Super::EndPlay(EndPlayReason);
}