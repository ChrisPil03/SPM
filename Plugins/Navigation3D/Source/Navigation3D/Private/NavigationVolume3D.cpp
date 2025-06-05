// Fill out your copyright notice in the Description page of Project Settings.

#include "NavigationVolume3D.h"
#include "NavNode.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Reverse.h"
#include "Async/Async.h"
#include "GameFramework/Actor.h"

#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"


ANavigationVolume3D::ANavigationVolume3D()
{
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
    ProceduralMesh->bHiddenInGame = true;

    ObstacleObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    ObstacleObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
}

bool ANavigationVolume3D::FindRandomValidLocationInRadius(
    const FVector& Origin,
    float WorldRadius,
    FVector& OutValidLocation,
    const AActor* ActorToIgnoreForLOS,
    int32 MaxCandidatesToCollect
) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindRandomValidLocationInRadius"));

    if (!bNodesInitializedAndFinalized || Nodes.IsEmpty() || WorldRadius < 0.0f || DivisionSize < KINDA_SMALL_NUMBER)
    {
        if (WorldRadius < KINDA_SMALL_NUMBER && bNodesInitializedAndFinalized && !Nodes.IsEmpty() && DivisionSize > KINDA_SMALL_NUMBER)
        {
            const FIntVector OriginCoords = ConvertLocationToCoordinates(Origin);
            const NavNode* OriginNode = GetConstNode(OriginCoords);
            if (OriginNode && OriginNode->bIsTraversable)
            {
                OutValidLocation = ConvertCoordinatesToLocation(OriginNode->Coordinates);
                FHitResult HitResult;
                FCollisionQueryParams CollisionParams;
                if (ActorToIgnoreForLOS && IsValid(ActorToIgnoreForLOS)) { CollisionParams.AddIgnoredActor(ActorToIgnoreForLOS); }
                
                UWorld* World = GetWorld();
                bool bHasLineOfSight = true;
                if (World) {
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

    UWorld* World = GetWorld();
    FCollisionQueryParams LOS_CollisionParams;
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
                        if (World) {
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
    
    TWeakObjectPtr<ANavigationVolume3D> WeakThis(this);
    TWeakObjectPtr<const AActor> WeakRequestingActor(RequestingActor);
    FString ActorName = RequestingActor ? RequestingActor->GetName() : TEXT("UnknownActor");
    
    uint32_t NewSearchID = AtomicPathfindingSearchIDCounter.fetch_add(1, std::memory_order_relaxed) + 1;
    if (NewSearchID == 0) {
        NewSearchID = AtomicPathfindingSearchIDCounter.fetch_add(1, std::memory_order_relaxed) + 1;
    }


    Async(EAsyncExecution::Thread, [WeakThis, WeakRequestingActor, StartLocation, DestinationLocation, NewSearchID, OnCompleteCallback, ActorName]() {
        ANavigationVolume3D* StrongThis = WeakThis.Get();
        if (!StrongThis)
        {
            AsyncTask(ENamedThreads::GameThread, [OnCompleteCallback]() {
                if (OnCompleteCallback.IsBound())
                {
                    OnCompleteCallback.ExecuteIfBound(ENavigationVolumeResult::ENVR_VolumeNotReady, TArray<FVector>());
                }
            });
            return;
        }
        
        FPathfindingInternalResultBundle ResultBundle = StrongThis->ExecutePathfindingOnThread(
            WeakThis, WeakRequestingActor, StartLocation, DestinationLocation, NewSearchID
        );
        
        AsyncTask(ENamedThreads::GameThread, [WeakThis, ResultBundle, OnCompleteCallback]() {
            ANavigationVolume3D* GameThreadStrongThis = WeakThis.Get();
            if (!GameThreadStrongThis)
            {
                if (OnCompleteCallback.IsBound())
                {
                    OnCompleteCallback.ExecuteIfBound(ENavigationVolumeResult::ENVR_VolumeNotReady, ResultBundle.PathPoints);
                }
                return;
            }

            if (OnCompleteCallback.IsBound())
            {
                OnCompleteCallback.ExecuteIfBound(ResultBundle.ResultCode, ResultBundle.PathPoints);
            }
            
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
    TWeakObjectPtr<ANavigationVolume3D> WeakThisForConstAccess,
    TWeakObjectPtr<const AActor> WeakRequestingActor,
    const FVector& StartLocation,
    const FVector& DestinationLocation,
    uint32_t PathSearchID)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_Total"));
    
    const AActor* RequestingActorPtr = WeakRequestingActor.Get();
    FString ActorNameForLogging = RequestingActorPtr ? RequestingActorPtr->GetName() : TEXT("UnknownOrInvalidActor");
    FPathfindingInternalResultBundle ResultBundle(ActorNameForLogging);
    
    if (!bNodesInitializedAndFinalized || Nodes.IsEmpty()) {
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - Nodes not initialized or empty. Actor: %s"), *ActorNameForLogging);
        ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_VolumeNotReady;
        return ResultBundle;
    }

    const uint32_t MySearchID = PathSearchID;
    
    if (MySearchID == 0) {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - SearchID is 0. This is unexpected. Resetting all node SearchIDs. Actor: %s"), *ActorNameForLogging);
        for (NavNode& Node : Nodes) {
            Node.SearchID_Pathfinding = 0;

            Node.FScore_Pathfinding = std::numeric_limits<float>::max();
            Node.GScore_Pathfinding = std::numeric_limits<float>::max();
            Node.CameFrom_Pathfinding = nullptr;
        }
    }


    NavNode* StartNodePtr = nullptr;
    NavNode* EndNodePtr   = nullptr;

    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_NodeConversionAndValidation"));
        StartNodePtr = GetNode(ConvertLocationToCoordinates(StartLocation));
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
            if (NodeToResolve->SearchID_Pathfinding != MySearchID) {
                NodeToResolve->SearchID_Pathfinding = MySearchID;
                NodeToResolve->GScore_Pathfinding = std::numeric_limits<float>::max();
                NodeToResolve->FScore_Pathfinding = std::numeric_limits<float>::max();
                NodeToResolve->CameFrom_Pathfinding = nullptr;
            }

            if (!NodeToResolve->bIsTraversable) {
                TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(bIsStartNode ? TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_ResolveBlockedStart") : TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_ResolveBlockedEnd"));
                float SearchRadius = 300.f; FVector FoundLocation;
                if (this->FindRandomValidLocationInRadius(OriginalWorldLocation, SearchRadius, FoundLocation, RequestingActorPtr)) {
                    NodeToResolve = GetNode(ConvertLocationToCoordinates(FoundLocation));
                    if (!NodeToResolve) { 
                        UE_LOG(LogTemp, Warning, TEXT("ExecutePathfindingOnThread: ResolveBlockedNode found location but GetNode returned null. Actor: %s"), *ActorNameForLogging);
                        return bIsStartNode ? ENavigationVolumeResult::ENVR_StartNodeBlocked : ENavigationVolumeResult::ENVR_EndNodeBlocked;
                    }
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
        
        if (bDrawPathfindingDebug) {
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
    std::unordered_set<NavNode*> ClosedSetForThisSearchInstance;

    auto HeuristicCost = [EndNodePtr](const NavNode* Node) -> float { if (!Node || !EndNodePtr) return std::numeric_limits<float>::max(); return FVector::Distance(FVector(EndNodePtr->Coordinates), FVector(Node->Coordinates)); };
    auto NeighborDistance = [](const NavNode* Node1, const NavNode* Node2) -> float { 
        if (!Node1 || !Node2) return std::numeric_limits<float>::max();
        float dx = static_cast<float>(FMath::Abs(Node1->Coordinates.X - Node2->Coordinates.X));
        float dy = static_cast<float>(FMath::Abs(Node1->Coordinates.Y - Node2->Coordinates.Y));
        float dz = static_cast<float>(FMath::Abs(Node1->Coordinates.Z - Node2->Coordinates.Z));
        return FMath::Sqrt(dx*dx + dy*dy + dz*dz);
    };

    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::ExecutePathfindingOnThread_AStar_Init"));
    
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
            
            if (!Current || Current->SearchID_Pathfinding != MySearchID) {
                 continue;
            }
            
            if (bDrawPathfindingDebug) {
                 AddDebugSphere_TaskLocal(ResultBundle.DebugSpheresToDraw_TaskLocal, ConvertCoordinatesToLocation(Current->Coordinates), DebugNodeSphereRadius * 1.2f, FColor::Yellow);
            }
            
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
                if (Current->GScore_Pathfinding == std::numeric_limits<float>::max()) { continue; }

                for (NavNode* Neighbor : Current->Neighbors) {
                    if (!Neighbor || !Neighbor->bIsTraversable) continue;

                    if (bDrawPathfindingDebug) {
                        AddDebugLine_TaskLocal(ResultBundle.DebugLinesToDraw_TaskLocal, ConvertCoordinatesToLocation(Current->Coordinates), ConvertCoordinatesToLocation(Neighbor->Coordinates), FColor(128,128,128,100), 0.5f);
                    }
                    
                    if (ClosedSetForThisSearchInstance.count(Neighbor)) {
                        continue;
                    }

                    float DistCurrentToNeighbor = NeighborDistance(Current, Neighbor);
                    if (DistCurrentToNeighbor == std::numeric_limits<float>::max()) continue;
                    
                    float TentativeGScore = Current->GScore_Pathfinding + DistCurrentToNeighbor;
                    
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
        while (PathNode != nullptr && PathNode->SearchID_Pathfinding == MySearchID) {
            TempPath.Add(ConvertCoordinatesToLocation(PathNode->Coordinates));
            PathNode = PathNode->CameFrom_Pathfinding;
        }
        Algo::Reverse(TempPath);
        ResultBundle.PathPoints = TempPath;
        // UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - Path successfully found. Actor: %s. Steps: %d"), *ActorNameForLogging, ResultBundle.PathPoints.Num());

        ResultBundle.bIsLongPath_TaskLocal = ResultBundle.PathPoints.Num() > LongPathThreshold;
        ResultBundle.bShouldDrawThisPathAndExploration_TaskLocal = bDrawPathfindingDebug && (ResultBundle.bIsLongPath_TaskLocal || !bOnlyDrawDebugForLongPaths);

        if (bDrawPathfindingDebug && ResultBundle.bShouldDrawThisPathAndExploration_TaskLocal) {
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
        
        if (ResultBundle.bIsLongPath_TaskLocal && bDrawPathfindingDebug) {
            UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - LONG PATH DETECTED: Actor: %s, %d steps (Threshold: %d)"), *ActorNameForLogging, ResultBundle.PathPoints.Num(), LongPathThreshold);
        }
        ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_Success;
        return ResultBundle;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ExecutePathfindingOnThread - Failed to find path. Actor: %s, StartNode: %s, EndNode: %s, StartLoc: %s, DestLoc: %s"),
        *ActorNameForLogging,
        (StartNodePtr ? *StartNodePtr->Coordinates.ToString() : TEXT("INVALID_START")),
        (EndNodePtr ? *EndNodePtr->Coordinates.ToString() : TEXT("INVALID_END")),
        *StartLocation.ToString(),
        *DestinationLocation.ToString());
    
    ResultBundle.bShouldDrawThisPathAndExploration_TaskLocal = bDrawPathfindingDebug && !bOnlyDrawDebugForLongPaths;
    ResultBundle.ResultCode = ENavigationVolumeResult::ENVR_NoPathExists;
    return ResultBundle;
}


void ANavigationVolume3D::AddDebugSphere_TaskLocal(TArray<FDebugSphereData>& DebugSpheresArray, const FVector& Center, float Radius, const FColor& InSphereColor, int32 Segments) const
{
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
    if (!World || !bDrawPathfindingDebug) 
    {
        return;
    }

    float ActualLifetime = Lifetime;

    for (const auto& SphereData : Spheres)
    {
        DrawDebugSphere(World, SphereData.Center, SphereData.Radius, SphereData.Segments, SphereData.Color, false, ActualLifetime, 0, SphereData.Radius / 10.f);
    }
    for (const auto& LineData : Lines)
    {
        DrawDebugLine(World, LineData.Start, LineData.End, LineData.Color, false, ActualLifetime, 0, LineData.Thickness);
    }
}

void ANavigationVolume3D::BeginPlay()
{
    Super::BeginPlay();

    if (bNodesInitializedAndFinalized) {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D (%s): BeginPlay - Nodes already initialized. Skipping."), *GetName());
        return;
    }

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

                } else {
                    UE_LOG(LogTemp, Fatal, TEXT("ANavigationVolume3D (%s): BeginPlay - GetNode returned nullptr for valid grid coordinate %s during Phase 1. This should not happen."), *GetName(), *CurrentCoords.ToString());
                    ensureMsgf(false, TEXT("GetNode returned null for an expected valid coordinate during node initialization."));
                    return;
                }
            }
        }
    }
    
    auto addNeighborIfValid = [&](NavNode* CenterNode, const FIntVector& NeighborGridCoords) {
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
    AtomicPathfindingSearchIDCounter.store(0);
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): BeginPlay - Initialization complete and finalized."), *GetName());
}

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
        const float GridSizeX_Local = static_cast<float>(DivisionsX) * DivisionSize;
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
    if (DivisionsX > 0) InOutCoordinates.X = FMath::Clamp(InOutCoordinates.X, 0, DivisionsX - 1);
    else InOutCoordinates.X = 0;

    if (DivisionsY > 0) InOutCoordinates.Y = FMath::Clamp(InOutCoordinates.Y, 0, DivisionsY - 1);
    else InOutCoordinates.Y = 0;

    if (DivisionsZ > 0) InOutCoordinates.Z = FMath::Clamp(InOutCoordinates.Z, 0, DivisionsZ - 1);
    else InOutCoordinates.Z = 0;
}

bool ANavigationVolume3D::AreCoordinatesValid(const FIntVector& Coordinates) const
{
    if (DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0) return false;
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
         return;
     }

     const FVector HalfBoxExtent(DivisionSize * 0.45f);
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
    const float HalfLineThickness = LineThickness * 0.5f;
    FVector LineDirection = End - Start;
    if (!LineDirection.Normalize()) return; 

    FVector ThicknessDir1 = FVector::CrossProduct(LineDirection, Normal).GetSafeNormal();
    if (ThicknessDir1.IsNearlyZero()) 
    {
         FVector NonParallelNormal = Normal.GetSafeNormal();
         if(FMath::Abs(FVector::DotProduct(LineDirection, NonParallelNormal)) > 0.999f)
         {
            LineDirection.FindBestAxisVectors(ThicknessDir1, NonParallelNormal);
         }
         ThicknessDir1 = FVector::CrossProduct(LineDirection, NonParallelNormal).GetSafeNormal();

         if (ThicknessDir1.IsNearlyZero()) ThicknessDir1 = FVector(0,1,0);
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
    if (DivisionSize < KINDA_SMALL_NUMBER || DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ConvertLocationToCoordinates - Invalid DivisionSize or Dimensions. Returning zero vector."));
        return FIntVector::ZeroValue;
    }

    FIntVector Coordinates;

    const FVector GridSpaceLocation = GetActorTransform().InverseTransformPosition(Location);
    
    Coordinates.X = FMath::FloorToInt(GridSpaceLocation.X / DivisionSize);
    Coordinates.Y = FMath::FloorToInt(GridSpaceLocation.Y / DivisionSize);
    Coordinates.Z = FMath::FloorToInt(GridSpaceLocation.Z / DivisionSize);
    
    FIntVector ClampedCoordinates = Coordinates;
    ClampCoordinates(ClampedCoordinates);
    return ClampedCoordinates;
}

FVector ANavigationVolume3D::ConvertCoordinatesToLocation(const FIntVector& Coordinates) const
{
    if (DivisionSize < KINDA_SMALL_NUMBER || DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::ConvertCoordinatesToLocation - Invalid DivisionSize or Dimensions. Returning zero vector."));
        return FVector::ZeroVector;
    }
    
    FIntVector ClampedCoordinates = Coordinates;

    ClampCoordinates(ClampedCoordinates);

    FVector GridSpaceLocation;

    GridSpaceLocation.X = (static_cast<float>(ClampedCoordinates.X) + 0.5f) * DivisionSize;
    GridSpaceLocation.Y = (static_cast<float>(ClampedCoordinates.Y) + 0.5f) * DivisionSize;
    GridSpaceLocation.Z = (static_cast<float>(ClampedCoordinates.Z) + 0.5f) * DivisionSize;
    
    return GetActorTransform().TransformPosition(GridSpaceLocation);
}

void ANavigationVolume3D::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): EndPlay called. Node count before empty: %d. Initialized: %s"), 
        *GetName(), Nodes.Num(), bNodesInitializedAndFinalized ? TEXT("true") : TEXT("false"));
    
    if (bNodesInitializedAndFinalized)
    {
        for (NavNode& Node : Nodes)
        {
            Node.Neighbors.clear();
        }
    }

    Nodes.Empty();

    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Nodes array emptied. Node count after empty: %d"), *GetName(), Nodes.Num());
    
    bNodesInitializedAndFinalized = false;
    Super::EndPlay(EndPlayReason);
}