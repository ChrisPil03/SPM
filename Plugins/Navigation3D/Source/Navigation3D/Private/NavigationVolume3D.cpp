// Fill out your copyright notice in the Description page of Project Settings.

#include "NavigationVolume3D.h" // Should be first for Precompiled Headers if used
#include "NavNode.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Reverse.h"

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
    int32 MaxCandidatesToCollect // This parameter is not effectively used with pick-first logic
) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindRandomValidLocationInRadius"));

    if (!bNodesInitializedAndFinalized || Nodes.IsEmpty() || WorldRadius < 0.0f || DivisionSize < KINDA_SMALL_NUMBER)
    {
        if (WorldRadius < KINDA_SMALL_NUMBER && bNodesInitializedAndFinalized && !Nodes.IsEmpty() && DivisionSize > KINDA_SMALL_NUMBER)
        {
            const FIntVector OriginCoords = ConvertLocationToCoordinates(Origin);
            const NavNode* OriginNode = GetConstNode(OriginCoords); // Use const version
            if (OriginNode && OriginNode->bIsTraversable)
            {
                OutValidLocation = ConvertCoordinatesToLocation(OriginNode->Coordinates);
                FHitResult HitResult;
                FCollisionQueryParams CollisionParams;
                if (ActorToIgnoreForLOS) { CollisionParams.AddIgnoredActor(ActorToIgnoreForLOS); }
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
    if (ActorToIgnoreForLOS) { LOS_CollisionParams.AddIgnoredActor(ActorToIgnoreForLOS); }
    FCollisionObjectQueryParams LOS_ObjectQueryParams(ObstacleObjectTypes);

    for (int32 dz = ExpandedSearchRadiusInCells; dz >= -ExpandedSearchRadiusInCells; --dz) {
        for (int32 dy = -ExpandedSearchRadiusInCells; dy <= ExpandedSearchRadiusInCells; ++dy) {
            for (int32 dx = -ExpandedSearchRadiusInCells; dx <= ExpandedSearchRadiusInCells; ++dx) {
                const FIntVector CurrentGridCoords = OriginGridCoords + FIntVector(dx, dy, dz);
                if (!AreCoordinatesValid(CurrentGridCoords)) continue;

                const NavNode* CandidateNode = GetConstNode(CurrentGridCoords); // Use const version
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

void ANavigationVolume3D::BeginPlay()
{
    Super::BeginPlay();

    if (bNodesInitializedAndFinalized) { // Prevent re-initialization
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D (%s): BeginPlay - Nodes already initialized. Skipping."), *GetName());
        return;
    }

    const int32 TotalNodes = GetTotalDivisions();
    if (TotalNodes <= 0) {
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D (%s): BeginPlay - Invalid dimensions (X=%d, Y=%d, Z=%d). Cannot initialize nodes."), *GetName(), DivisionsX, DivisionsY, DivisionsZ);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): BeginPlay - Initializing %d nodes."), *GetName(), TotalNodes);

    // Size the array ONCE. This is the critical step for pointer stability.
    Nodes.Empty(TotalNodes); // Reserve capacity and clear any old data.
    Nodes.SetNum(TotalNodes); // Allocate and default-construct all NavNode objects.
                              // From this point, pointers to elements in 'Nodes' will be stable
                              // UNLESS 'Nodes' itself is modified in a way that forces reallocation (Add, Insert, SetNum > capacity, etc.).

    // Phase 1: Initialize basic node data (Coordinates, Traversability default)
    for (int32 z = 0; z < DivisionsZ; ++z) {
        for (int32 y = 0; y < DivisionsY; ++y) {
            for (int32 x = 0; x < DivisionsX; ++x) {
                FIntVector CurrentCoords(x, y, z);
                NavNode* CurrentNodePtr = GetNode(CurrentCoords); // Use non-const
                if (CurrentNodePtr) {
                    CurrentNodePtr->Coordinates = CurrentCoords;
                    CurrentNodePtr->bIsTraversable = true;
                    CurrentNodePtr->Neighbors.clear(); // Ensure it's empty before populating
                    // A* specific data is default initialized by NavNode constructor
                } else {
                    UE_LOG(LogTemp, Fatal, TEXT("ANavigationVolume3D (%s): BeginPlay - GetNode returned nullptr for valid grid coordinate %s during Phase 1. This should not happen."), *GetName(), *CurrentCoords.ToString());
                    // If this ensure fires, there's a fundamental issue with GetNode or grid dimensions vs. Nodes.Num()
                    ensureMsgf(false, TEXT("GetNode returned null for an expected valid coordinate during node initialization."));
                    return; // Cannot proceed
                }
            }
        }
    }

    // Phase 2: Populate Neighbors (now that all NavNode objects exist at stable memory locations)
    auto addNeighborIfValid = [&](NavNode* CenterNode, const FIntVector& NeighborGridCoords) {
        if (!CenterNode) return;
        if (AreCoordinatesValid(NeighborGridCoords)) {
            int32 SharedAxes = 0;
            if (CenterNode->Coordinates.X == NeighborGridCoords.X) SharedAxes++;
            if (CenterNode->Coordinates.Y == NeighborGridCoords.Y) SharedAxes++;
            if (CenterNode->Coordinates.Z == NeighborGridCoords.Z) SharedAxes++;

            if (SharedAxes >= MinSharedNeighborAxes && SharedAxes < 3) { // SharedAxes < 3 avoids self
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

    for (NavNode& CurrentNode : Nodes) { // Iterate by reference
        // CurrentNode.Neighbors.clear(); // Already done in Phase 1, but doesn't hurt.
        for (int dz = -1; dz <= 1; ++dz) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0 && dz == 0) continue; // Skip self
                    addNeighborIfValid(&CurrentNode, CurrentNode.Coordinates + FIntVector(dx, dy, dz));
                }
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): BeginPlay - Node neighbor population finished."), *GetName());

    PrecomputeNodeTraversability();
    bNodesInitializedAndFinalized = true;
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): BeginPlay - Initialization complete and finalized."), *GetName());
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

void ANavigationVolume3D::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// --- Pathfinding Implementation ---
ENavigationVolumeResult ANavigationVolume3D::FindPath(const AActor* Actor, const FVector& StartLocation, const FVector& DestinationLocation, TArray<FVector>& OutPath)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPath_Total"));
    OutPath.Empty();
    DebugSpheresToDraw.Empty(); // Clear debug data from any previous calls
    DebugLinesToDraw.Empty();

    UWorld* World = GetWorld(); // Get world once for potential drawing

    if (!bNodesInitializedAndFinalized || Nodes.IsEmpty()) {
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D::FindPath - Nodes not initialized or empty. Cannot pathfind."));
        return ENavigationVolumeResult::ENVR_UnknownError;
    }

    CurrentPathfindingSearchID++;
    if (CurrentPathfindingSearchID == 0) {
        CurrentPathfindingSearchID = 1;
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - SearchID wrapped. Forcing full NavNode A* data reset."));
        for (NavNode& Node : Nodes) {
            Node.SearchID_Pathfinding = 0;
            Node.FScore_Pathfinding = std::numeric_limits<float>::max();
            Node.GScore_Pathfinding = std::numeric_limits<float>::max();
            Node.CameFrom_Pathfinding = nullptr;
        }
    }
    const uint32_t MySearchID = CurrentPathfindingSearchID;

    NavNode* StartNodePtr = nullptr;
    NavNode* EndNodePtr   = nullptr;

    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPath_NodeConversionAndValidation"));
        StartNodePtr = GetNode(ConvertLocationToCoordinates(StartLocation));
        EndNodePtr   = GetNode(ConvertLocationToCoordinates(DestinationLocation));

        if (!StartNodePtr) { UE_LOG(LogTemp, Warning, TEXT("FindPath: StartNodePtr is null for StartLocation: %s"), *StartLocation.ToString()); return ENavigationVolumeResult::ENVR_StartNodeInvalid; }
        if (!EndNodePtr) { UE_LOG(LogTemp, Warning, TEXT("FindPath: EndNodePtr is null for DestinationLocation: %s"), *DestinationLocation.ToString()); return ENavigationVolumeResult::ENVR_EndNodeInvalid; }

        auto ResolveBlockedNode = [&](NavNode*& NodeToResolve, const FVector& OriginalWorldLocation, bool bIsStartNode) -> ENavigationVolumeResult {
            if (NodeToResolve->SearchID_Pathfinding != MySearchID) {
                NodeToResolve->SearchID_Pathfinding = MySearchID;
                NodeToResolve->GScore_Pathfinding = std::numeric_limits<float>::max();
                NodeToResolve->FScore_Pathfinding = std::numeric_limits<float>::max();
                NodeToResolve->CameFrom_Pathfinding = nullptr;
            }
            if (!NodeToResolve->bIsTraversable) {
                TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(bIsStartNode ? TEXT("ANavigationVolume3D::FindPath_ResolveBlockedStart") : TEXT("ANavigationVolume3D::FindPath_ResolveBlockedEnd"));
                float SearchRadius = 300.f; FVector FoundLocation;
                if (FindRandomValidLocationInRadius(OriginalWorldLocation, SearchRadius, FoundLocation, Actor)) {
                    NodeToResolve = GetNode(ConvertLocationToCoordinates(FoundLocation));
                    if (!NodeToResolve) { UE_LOG(LogTemp, Warning, TEXT("FindPath: ResolveBlockedNode found location but GetNode returned null.")); return bIsStartNode ? ENavigationVolumeResult::ENVR_StartNodeBlocked : ENavigationVolumeResult::ENVR_EndNodeBlocked; }
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
        if(StartResolveResult != ENavigationVolumeResult::ENVR_Success) return StartResolveResult;
        
        ENavigationVolumeResult EndResolveResult = ResolveBlockedNode(EndNodePtr, DestinationLocation, false);
        if(EndResolveResult != ENavigationVolumeResult::ENVR_Success) return EndResolveResult;
        
        // Add initial Start/End spheres to the debug draw list
        AddDebugSphere(ConvertCoordinatesToLocation(StartNodePtr->Coordinates), DebugNodeSphereRadius * 1.5f, FColor::Cyan, 12);
        AddDebugSphere(ConvertCoordinatesToLocation(EndNodePtr->Coordinates), DebugNodeSphereRadius * 1.5f, FColor::Magenta, 12);
        
        if (StartNodePtr == EndNodePtr) { 
            TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPath_PathToSelf"));
            OutPath.Add(ConvertCoordinatesToLocation(StartNodePtr->Coordinates));
            UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D::FindPath - Path to self found. Path steps: %d"), OutPath.Num());
            if (bDrawPathfindingDebug && (!bOnlyDrawDebugForLongPaths || OutPath.Num() > LongPathThreshold) ) { // Check long path condition even for path to self
                FlushDebugDraws(World, DebugDrawLifetime);
            }
            return ENavigationVolumeResult::ENVR_PathToSelf;
        }
    }

    std::priority_queue<NavNode*, std::vector<NavNode*>, NodeCompare> OpenSet;
    std::unordered_set<NavNode*> ClosedSetForThisSearch; 

    auto HeuristicCost = [EndNodePtr](const NavNode* Node) -> float { if (!Node || !EndNodePtr) return std::numeric_limits<float>::max(); return FVector::Distance(FVector(EndNodePtr->Coordinates), FVector(Node->Coordinates)); };
    auto NeighborDistance = [](const NavNode* Node1, const NavNode* Node2) -> float { 
        if (!Node1 || !Node2) return std::numeric_limits<float>::max();
        float dx = static_cast<float>(FMath::Abs(Node1->Coordinates.X - Node2->Coordinates.X));
        float dy = static_cast<float>(FMath::Abs(Node1->Coordinates.Y - Node2->Coordinates.Y));
        float dz = static_cast<float>(FMath::Abs(Node1->Coordinates.Z - Node2->Coordinates.Z));
        return FMath::Sqrt(dx*dx + dy*dy + dz*dz);
    };

    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPath_AStar_Init"));
        ensureMsgf(StartNodePtr->SearchID_Pathfinding == MySearchID, TEXT("StartNodePtr not initialized for current SearchID before A* Init!"));
        StartNodePtr->GScore_Pathfinding = 0.0f;
        StartNodePtr->FScore_Pathfinding = HeuristicCost(StartNodePtr);
        OpenSet.push(StartNodePtr);
        AddDebugSphere(ConvertCoordinatesToLocation(StartNodePtr->Coordinates), DebugNodeSphereRadius, FColor::Green);
    }

    bool bPathSuccessfullyFound = false;
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPath_AStar_MainLoop"));
        while (!OpenSet.empty())
        {
            NavNode* Current = OpenSet.top(); OpenSet.pop();
            if (!Current || Current->SearchID_Pathfinding != MySearchID) { continue; }
            
            AddDebugSphere(ConvertCoordinatesToLocation(Current->Coordinates), DebugNodeSphereRadius * 1.2f, FColor::Yellow);
            
            if (ClosedSetForThisSearch.contains(Current)) { continue; }
            ClosedSetForThisSearch.insert(Current);

            AddDebugSphere(ConvertCoordinatesToLocation(Current->Coordinates), DebugNodeSphereRadius, FColor::Red);
            
            if (Current == EndNodePtr) {
                bPathSuccessfullyFound = true;
                break; 
            }

            {
                TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPath_NeighborLoop"));
                if (Current->GScore_Pathfinding == std::numeric_limits<float>::max()) { continue; }

                for (NavNode* Neighbor : Current->Neighbors) {
                    if (!Neighbor || !Neighbor->bIsTraversable) continue;
                    AddDebugLine(ConvertCoordinatesToLocation(Current->Coordinates), ConvertCoordinatesToLocation(Neighbor->Coordinates), FColor(128,128,128,100), 0.5f);
                    if (ClosedSetForThisSearch.contains(Neighbor)) continue;

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
                        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPath_UpdateNeighborPath"));
                        Neighbor->CameFrom_Pathfinding = Current;
                        Neighbor->GScore_Pathfinding = TentativeGScore;
                        Neighbor->FScore_Pathfinding = TentativeGScore + HeuristicCost(Neighbor); 
                        OpenSet.push(Neighbor);
                        AddDebugSphere(ConvertCoordinatesToLocation(Neighbor->Coordinates), DebugNodeSphereRadius, FColor::Green);
                        AddDebugLine(ConvertCoordinatesToLocation(Current->Coordinates), ConvertCoordinatesToLocation(Neighbor->Coordinates), FColor::White);
                    }
                }
            }
        }
    }

    if (bPathSuccessfullyFound) {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ANavigationVolume3D::FindPath_PathReconstruction"));
        TArray<FVector> TempPath;
        NavNode* PathNode = EndNodePtr;
        while (PathNode != nullptr && PathNode->SearchID_Pathfinding == MySearchID) {
            TempPath.Add(ConvertCoordinatesToLocation(PathNode->Coordinates));
            PathNode = PathNode->CameFrom_Pathfinding;
        }
        Algo::Reverse(TempPath);
        OutPath = TempPath;
        UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D::FindPath - Path successfully found. Number of steps: %d"), OutPath.Num());

        bool bIsLongPath = OutPath.Num() > LongPathThreshold;
        // Determine if we should draw based on master flag and (long path OR not only_long_paths mode)
        bool bActuallyDrawThisPathAndExploration = bDrawPathfindingDebug && (bIsLongPath || !bOnlyDrawDebugForLongPaths);

        if (bActuallyDrawThisPathAndExploration) {
            // Add final path visuals to the debug draw list
            NavNode* VisPathNode = EndNodePtr;
            while (VisPathNode != nullptr && VisPathNode->SearchID_Pathfinding == MySearchID) {
                FVector NodeLoc = ConvertCoordinatesToLocation(VisPathNode->Coordinates);
                // Use a distinct color for the final path nodes if desired, or keep it Cyan
                AddDebugSphere(NodeLoc, DebugNodeSphereRadius * 0.9f, FColorList::NeonPink, 10); // Brighter, more distinct for final path
                if (VisPathNode->CameFrom_Pathfinding && VisPathNode->CameFrom_Pathfinding->SearchID_Pathfinding == MySearchID) {
                    AddDebugLine(NodeLoc, ConvertCoordinatesToLocation(VisPathNode->CameFrom_Pathfinding->Coordinates), FColorList::NeonPink, 3.5f);
                }
                VisPathNode = VisPathNode->CameFrom_Pathfinding;
            }
            FlushDebugDraws(World, bIsLongPath ? DebugDrawLifetime * 2.0f : DebugDrawLifetime);
        } else {
             // Ensure lists are cleared even if not drawn this time due to bOnlyDrawDebugForLongPaths
            DebugSpheresToDraw.Empty();
            DebugLinesToDraw.Empty();
        }

        if (bIsLongPath && bDrawPathfindingDebug) { // Separate check for logging and pausing
            UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - LONG PATH DETECTED: %d steps (Threshold: %d)"), OutPath.Num(), LongPathThreshold);
            if (bPauseOnLongPath && World) {
                UGameplayStatics::SetGamePaused(World, true);
                UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - Game PAUSED due to long path."));
            }
        }
        return ENavigationVolumeResult::ENVR_Success;
    }

    // Path not found
    UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - %s failed to find path from node %s to %s. At start position: %s and end position: %s"),
        *Actor->GetName(),
        (StartNodePtr ? *StartNodePtr->Coordinates.ToString() : TEXT("INVALID_START")),
        (EndNodePtr ? *EndNodePtr->Coordinates.ToString() : TEXT("INVALID_END")),
        *StartLocation.ToString(),
        *DestinationLocation.ToString())
    // If general debug drawing is on (not filtered by long paths) and path failed, draw what was explored
    if (bDrawPathfindingDebug && !bOnlyDrawDebugForLongPaths) {
        FlushDebugDraws(World, DebugDrawLifetime);
    } else {
        DebugSpheresToDraw.Empty(); // Still clear if not drawing
        DebugLinesToDraw.Empty();
    }
    return ENavigationVolumeResult::ENVR_NoPathExists;
}

// Add these helper functions (can be at the bottom or in a sensible place)
void ANavigationVolume3D::AddDebugSphere(const FVector& Center, float Radius, const FColor& InSphereColor, int32 Segments)
{
    if (!bDrawPathfindingDebug) return; // Check master flag
    DebugSpheresToDraw.Add({Center, Radius, InSphereColor, Segments});
}

void ANavigationVolume3D::AddDebugLine(const FVector& Start, const FVector& End, const FColor& InLineColor, float Thickness)
{
    if (!bDrawPathfindingDebug) return; // Check master flag
    DebugLinesToDraw.Add({Start, End, InLineColor, Thickness});
}

void ANavigationVolume3D::FlushDebugDraws(UWorld* World, float Lifetime)
{
    if (!World || !bDrawPathfindingDebug) // Check master flag again
    {
        DebugSpheresToDraw.Empty();
        DebugLinesToDraw.Empty();
        return;
    }

    for (const auto& SphereData : DebugSpheresToDraw)
    {
        DrawDebugSphere(World, SphereData.Center, SphereData.Radius, SphereData.Segments, SphereData.Color, false, Lifetime, 0, SphereData.Radius / 10.f); // Thickness based on radius
    }
    for (const auto& LineData : DebugLinesToDraw)
    {
        DrawDebugLine(World, LineData.Start, LineData.End, LineData.Color, false, Lifetime, 0, LineData.Thickness);
    }
    DebugSpheresToDraw.Empty();
    DebugLinesToDraw.Empty();
}


// Ensure GetNode (non-const) and GetConstNode (const) implementations are robust
NavNode* ANavigationVolume3D::GetNode(FIntVector Coordinates) {
    FIntVector ClampedCoords = Coordinates;
    ClampCoordinates(ClampedCoords); // ClampCoordinates is const, can be called. It modifies ClampedCoords.

    if (DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0) {
        // UE_LOG(LogTemp, Warning, TEXT("GetNode: Invalid divisions X%d Y%d Z%d"), DivisionsX, DivisionsY, DivisionsZ);
        return nullptr;
    }
    if (Nodes.IsEmpty()) { // Check if Nodes array has even been initialized
        // UE_LOG(LogTemp, Warning, TEXT("GetNode: Nodes array is empty."));
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

// ... (ClampCoordinates, AreCoordinatesValid, PrecomputeNodeTraversability, CreateLine) ...
// Ensure ClampCoordinates and AreCoordinatesValid are robust.
// Your PrecomputeNodeTraversability and CreateLine seem fine as they were.

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