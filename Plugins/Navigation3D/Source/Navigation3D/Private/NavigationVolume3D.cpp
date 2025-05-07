// Fill out your copyright notice in the Description page of Project Settings.

#include "NavigationVolume3D.h"
#include "NavNode.h" // Include NavNode definition
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Reverse.h" // For path reconstruction

// Standard library includes needed for A* implementation as written
#include <set>
#include <unordered_map>
#include <limits> // For FLT_MAX

// Sets default values
ANavigationVolume3D::ANavigationVolume3D()
{
    // Set this actor to call Tick() every frame. Set false for performance if Tick is not needed.
    PrimaryActorTick.bCanEverTick = false; // *** Disabled Tick by default ***

    DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>("DefaultSceneComponent");
    SetRootComponent(DefaultSceneComponent);

    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
    ProceduralMesh->SetupAttachment(GetRootComponent());
    ProceduralMesh->CastShadow = false;
    ProceduralMesh->SetEnableGravity(false);
    ProceduralMesh->bApplyImpulseOnDamage = false;
    ProceduralMesh->SetGenerateOverlapEvents(false);
    ProceduralMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
    ProceduralMesh->SetCollisionProfileName(FName("NoCollision")); // Use FName constructor
    ProceduralMesh->bHiddenInGame = true; // Hide visualization mesh during gameplay

    // Initialize ObstacleObjectTypes (Example: WorldStatic and WorldDynamic)
    ObstacleObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    ObstacleObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    // ObstacleActorClassFilter defaults to nullptr (no class filter)

    // GridMaterial is assigned via UPROPERTY in editor, no need for constructor finder
}

void ANavigationVolume3D::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Ensure valid dimensions
    DivisionsX = FMath::Max(1, DivisionsX);
    DivisionsY = FMath::Max(1, DivisionsY);
    DivisionsZ = FMath::Max(1, DivisionsZ);
    DivisionSize = FMath::Max(1.0f, DivisionSize);
    LineThickness = FMath::Max(0.1f, LineThickness);

    if (bDrawGridLinesInEditor) // *** ADD THIS CHECK ***
    {
        // Create arrays for mesh geometry
        TArray<FVector> Vertices;
        TArray<int32> Triangles;

        // Define grid boundaries for line drawing
        const float GridSizeX = GetGridSizeX();
        const float GridSizeY = GetGridSizeY();
        const float GridSizeZ = GetGridSizeZ();
        FVector Start, End;

        // --- Create Grid Line Geometry ---
        // X-Axis aligned lines (running along Y)
        for (int32 z = 0; z <= DivisionsZ; ++z) {
            for (int32 x = 0; x <= DivisionsX; ++x) {
                Start.Set(x * DivisionSize, 0.0f, z * DivisionSize);
                End.Set(x * DivisionSize, GridSizeY, z * DivisionSize);
                CreateLine(Start, End, FVector::RightVector, Vertices, Triangles); // Normal relative to line direction
            }
        }
        // Y-Axis aligned lines (running along X)
        for (int32 z = 0; z <= DivisionsZ; ++z) {
            for (int32 y = 0; y <= DivisionsY; ++y) {
                Start.Set(0.0f, y * DivisionSize, z * DivisionSize);
                End.Set(GridSizeX, y * DivisionSize, z * DivisionSize);
                CreateLine(Start, End, FVector::ForwardVector, Vertices, Triangles); // Normal relative to line direction
            }
        }
        // Z-Axis aligned lines (running along Z)
        for (int32 x = 0; x <= DivisionsX; ++x) {
            for (int32 y = 0; y <= DivisionsY; ++y) {
                Start.Set(x * DivisionSize, y * DivisionSize, 0.0f);
                End.Set(x * DivisionSize, y * DivisionSize, GridSizeZ);
                CreateLine(Start, End, FVector::UpVector, Vertices, Triangles); // Normal relative to line direction
            }
        }
        // --- End Grid Line Geometry ---

        // Required but unused parameters for CreateMeshSection
        TArray<FVector> Normals;
        TArray<FVector2D> UVs;
        TArray<FColor> Colors;
        TArray<FProcMeshTangent> Tangents;

        // Update the procedural mesh component
        ProceduralMesh->ClearMeshSection(0); // Clear previous geometry
        if(Vertices.Num() > 0 && Triangles.Num() > 0)
        {
            ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, false);
        }

        // Apply the assigned material
        if (IsValid(GridMaterial))
        {
            // Use existing MID or create one
            UMaterialInstanceDynamic* DynamicMaterialInstance = Cast<UMaterialInstanceDynamic>(ProceduralMesh->GetMaterial(0));
            if (!DynamicMaterialInstance || DynamicMaterialInstance->GetBaseMaterial() != GridMaterial)
            {
                 DynamicMaterialInstance = ProceduralMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, GridMaterial);
            }

            if (DynamicMaterialInstance)
            {
                DynamicMaterialInstance->SetVectorParameterValue(FName("Color"), Color);
                // Optional: If material has separate Opacity scalar param:
                // DynamicMaterialInstance->SetScalarParameterValue(FName("Opacity"), Color.A);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D (%s): Failed to create/set dynamic material instance."), *GetName());
                 ProceduralMesh->SetMaterial(0, GridMaterial); // Fallback
            }
        }
        else
        {
            // UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): GridMaterial property not assigned in editor."), *GetName());
            ProceduralMesh->SetMaterial(0, nullptr); // No material if none assigned
        }
    }
    else // *** ELSE: Clear the mesh if not drawing ***
    {
        ProceduralMesh->ClearMeshSection(0);
        ProceduralMesh->SetMaterial(0, nullptr); // Ensure no material is lingering
    }
}

// Called when the game starts or when spawned
void ANavigationVolume3D::BeginPlay()
{
    Super::BeginPlay();

    // --- Initialize Node Array ---
    const int32 TotalNodes = GetTotalDivisions();
    if (TotalNodes <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D (%s): Invalid dimensions (X=%d, Y=%d, Z=%d), cannot initialize nodes."), *GetName(), DivisionsX, DivisionsY, DivisionsZ);
        return;
    }

    Nodes.Empty(TotalNodes); // Ensure clean state and reserve memory
    Nodes.SetNum(TotalNodes);

    // --- Helper Lambda for Adding Neighbors ---
    auto addNeighborIfValid = [&](NavNode* Node, const FIntVector& NeighborCoords)
    {
        if (!Node) return; // Safety check

        // Check if neighbor coordinates are within grid bounds
        if (AreCoordinatesValid(NeighborCoords))
        {
            // Check shared axes requirement
            int32 SharedAxes = 0;
            if (Node->Coordinates.X == NeighborCoords.X) ++SharedAxes;
            if (Node->Coordinates.Y == NeighborCoords.Y) ++SharedAxes;
            if (Node->Coordinates.Z == NeighborCoords.Z) ++SharedAxes;

            // MinSharedNeighborAxes = 0 -> corners only
            // MinSharedNeighborAxes = 1 -> faces and edges
            // MinSharedNeighborAxes = 2 -> edges and corners only
            // SharedAxes < 3 ensures we don't add self
            if (SharedAxes >= MinSharedNeighborAxes && SharedAxes < 3)
            {
                NavNode* NeighborNodePtr = GetNode(NeighborCoords); // Get pointer to neighbor
                if (NeighborNodePtr)
                {
                    Node->Neighbors.push_back(NeighborNodePtr);
                }
                 else { // Should not happen if AreCoordinatesValid passed
                     UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D (%s): Failed to get valid neighbor node pointer for valid coordinates %s."), *GetName(), *NeighborCoords.ToString());
                 }
            }
        }
    };

    // --- Initialize Each Node ---
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Initializing %d nodes..."), *GetName(), TotalNodes);
    for (int32 z = 0; z < DivisionsZ; ++z) {
        for (int32 y = 0; y < DivisionsY; ++y) {
            for (int32 x = 0; x < DivisionsX; ++x) {
                FIntVector CurrentCoords(x, y, z);
                NavNode* CurrentNodePtr = GetNode(CurrentCoords); // Get pointer TO node

                if (CurrentNodePtr != nullptr)
                {
                    // Initialize node data
                    CurrentNodePtr->Coordinates = CurrentCoords;
                    CurrentNodePtr->Neighbors.clear(); // Reset neighbors
                    CurrentNodePtr->FScore = FLT_MAX;
                    CurrentNodePtr->bIsTraversable = true; // Assume traversable until checked

                    // Find neighbors by checking 3x3x3 cube around current node
                    for (int dz = -1; dz <= 1; ++dz) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            for (int dx = -1; dx <= 1; ++dx) {
                                // Skip self (0,0,0 offset)
                                if (dx == 0 && dy == 0 && dz == 0) {
                                    continue;
                                }
                                addNeighborIfValid(CurrentNodePtr, FIntVector(x + dx, y + dy, z + dz));
                            }
                        }
                    }
                }
                else { // Error getting node pointer
                    UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D (%s): GetNode returned null for loop coordinates %s during initialization."), *GetName(), *CurrentCoords.ToString());
                }
            } // End X loop
        } // End Y loop
    } // End Z loop
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Node initialization finished."), *GetName());

    // Pre-calculate traversability based on overlaps
    PrecomputeNodeTraversability();
}

void ANavigationVolume3D::PrecomputeNodeTraversability()
{
     UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Precomputing node traversability..."), *GetName());
     int32 NonTraversableCount = 0;

     // Only check if filter criteria are actually set
     if(ObstacleObjectTypes.Num() == 0 && ObstacleActorClassFilter == nullptr)
     {
         UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D (%s): No ObstacleObjectTypes or ObstacleActorClassFilter specified. Skipping traversability checks."), *GetName());
         return;
     }

     const FVector HalfBoxExtent(DivisionSize * 0.5f); // Half extent for box overlap
     TArray<AActor*> ActorsToIgnore; // Typically empty unless specific actors should be ignored
     TArray<AActor*> OutActors;

     for (NavNode& NodeToCheck : Nodes) // Iterate by reference
     {
         const FVector WorldLocation = ConvertCoordinatesToLocation(NodeToCheck.Coordinates);
         OutActors.Empty(); // Reuse array

         bool bOverlapped = UKismetSystemLibrary::BoxOverlapActors(
             this, // World context object
             WorldLocation,
             HalfBoxExtent,
             ObstacleObjectTypes,
             ObstacleActorClassFilter,
             ActorsToIgnore,
             OutActors
         );

         NodeToCheck.bIsTraversable = !bOverlapped; // Node is traversable if NOT overlapped
         if (!NodeToCheck.bIsTraversable) {
             NonTraversableCount++;
         }
     }
     UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Traversability precomputation complete. Found %d non-traversable nodes."), *GetName(), NonTraversableCount);
}


void ANavigationVolume3D::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Nodes.Empty(); // TArray manages memory
    UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D (%s): Nodes array cleared."), *GetName());
    Super::EndPlay(EndPlayReason);
}

// Called every frame (currently disabled by default)
void ANavigationVolume3D::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// --- Pathfinding Implementation ---

ENavigationVolumeResult ANavigationVolume3D::FindPath(const FVector& StartLocation, const FVector& DestinationLocation, TArray<FVector>& OutPath) const // Marked const
{
    OutPath.Empty();

    if (Nodes.IsEmpty()) {
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D::FindPath - Node array is empty. Cannot find path."));
        return ENavigationVolumeResult::ENVR_UnknownError;
    }

    // Need non-const access to GetNode because it wasn't marked const. Use const_cast as a workaround.
    // A better solution might involve const/non-const versions of GetNode.
    NavNode* StartNodePtr = const_cast<ANavigationVolume3D*>(this)->GetNode(ConvertLocationToCoordinates(StartLocation));
    NavNode* EndNodePtr   = const_cast<ANavigationVolume3D*>(this)->GetNode(ConvertLocationToCoordinates(DestinationLocation));

    // Validate start and end nodes
    if (!StartNodePtr) {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - Start location is outside volume bounds or invalid."));
        return ENavigationVolumeResult::ENVR_StartNodeInvalid;
    }
    if (!EndNodePtr) {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - Destination location is outside volume bounds or invalid."));
        return ENavigationVolumeResult::ENVR_EndNodeBlocked;
    }
    if (!StartNodePtr->bIsTraversable) {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - Start node %s is blocked."), *StartNodePtr->Coordinates.ToString());
        return ENavigationVolumeResult::ENVR_StartNodeBlocked;
    } 
    if (!EndNodePtr->bIsTraversable) {
        UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - Destination node %s is blocked."), *EndNodePtr->Coordinates.ToString());
        return ENavigationVolumeResult::ENVR_EndNodeBlocked;
    }
    if (StartNodePtr == EndNodePtr) { // Path to self
        OutPath.Add(ConvertCoordinatesToLocation(StartNodePtr->Coordinates));
        UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D::FindPath - Start and Destination are the same node."));
        return ENavigationVolumeResult::ENVR_PathToSelf;
    }


    // --- A* Data Structures ---
    std::multiset<NavNode*, NodeCompare> OpenSet;       // Nodes to visit, sorted by FScore
    std::unordered_map<NavNode*, NavNode*> CameFromMap; // Parent node in the best path found so far
    std::unordered_map<NavNode*, float> GScoreMap;      // Cost from start to this node

    // --- A* Heuristic & Cost Functions ---
    auto HeuristicCost = [EndNodePtr](const NavNode* Node) -> float {
        if (!Node || !EndNodePtr) return FLT_MAX;
        // Euclidean distance (sqrt is relatively expensive, could use squared distance if comparison allows)
        // Using integer coordinates for distance calculation assumes uniform grid cost.
        return FVector::Distance(FVector(EndNodePtr->Coordinates), FVector(Node->Coordinates));
    };
    auto NeighborDistance = [](const NavNode* Node1, const NavNode* Node2) -> float {
        if (!Node1 || !Node2) return FLT_MAX;
        // Calculate distance based on coordinate difference (diagonal costs more)
        float dx = FMath::Abs(Node1->Coordinates.X - Node2->Coordinates.X);
        float dy = FMath::Abs(Node1->Coordinates.Y - Node2->Coordinates.Y);
        float dz = FMath::Abs(Node1->Coordinates.Z - Node2->Coordinates.Z);
        // Simple Euclidean distance between centers based on grid coordinates
        return FMath::Sqrt(dx*dx + dy*dy + dz*dz);
        // Could multiply by DivisionSize here for world distance, but relative costs are often enough.
    };
    auto GetGScore = [&GScoreMap](const NavNode* Node) -> float {
         if (!Node) return FLT_MAX;
         auto Iter = GScoreMap.find(const_cast<NavNode*>(Node)); // Map might need non-const key
         return (Iter != GScoreMap.end()) ? Iter->second : FLT_MAX;
    };


    // --- Initialize A* ---
    // Reset FScore for start node (others default to FLT_MAX in NavNode struct)
    StartNodePtr->FScore = HeuristicCost(StartNodePtr);
    GScoreMap[StartNodePtr] = 0.0f;
    OpenSet.insert(StartNodePtr);

    // --- A* Main Loop ---
    while (!OpenSet.empty())
    {
        // Get node with lowest FScore
        NavNode* Current = *OpenSet.begin();
        OpenSet.erase(OpenSet.begin());

        // --- Goal Reached? ---
        if (Current == EndNodePtr)
        {
            // Reconstruct path
            TArray<FVector> TempPath;
            NavNode* PathNode = EndNodePtr;
            while (PathNode != nullptr) {
                TempPath.Add(ConvertCoordinatesToLocation(PathNode->Coordinates));
                auto Iter = CameFromMap.find(PathNode);
                PathNode = (Iter != CameFromMap.end()) ? Iter->second : nullptr; // Move to parent
            }
            Algo::Reverse(TempPath); // Reverse to Start -> End order
            OutPath = TempPath;
            UE_LOG(LogTemp, Log, TEXT("ANavigationVolume3D::FindPath - Path found with %d steps."), OutPath.Num());
            return ENavigationVolumeResult::ENVR_Success;
        }

        // --- Explore Neighbors ---
        for (NavNode* Neighbor : Current->Neighbors)
        {
            if (!Neighbor || !Neighbor->bIsTraversable) { // Skip null or blocked neighbors
                continue;
            }

            float TentativeGScore = GetGScore(Current) + NeighborDistance(Current, Neighbor);

            if (TentativeGScore < GetGScore(Neighbor))
            {
                // Better path found to this neighbor
                CameFromMap[Neighbor] = Current;
                GScoreMap[Neighbor] = TentativeGScore;
                Neighbor->FScore = TentativeGScore + HeuristicCost(Neighbor);

                // Add/Update neighbor in OpenSet. std::multiset handles duplicates & sorting.
                // A more optimized A* might check if already present and update, but insert works.
                OpenSet.insert(Neighbor);
            }
        } // End neighbor loop
    } // End while OpenSet not empty

    // Failed to find path - OpenSet emptied before goal was reached
    UE_LOG(LogTemp, Warning, TEXT("ANavigationVolume3D::FindPath - Failed to find path from node %s to %s."), *StartNodePtr->Coordinates.ToString(), *EndNodePtr->Coordinates.ToString());
    return ENavigationVolumeResult::ENVR_NoPathExists;
}


// --- Coordinate Conversion and Validation ---

FIntVector ANavigationVolume3D::ConvertLocationToCoordinates(const FVector& Location) const
{
    FIntVector Coordinates;
    const FVector GridSpaceLocation = UKismetMathLibrary::InverseTransformLocation(GetActorTransform(), Location);

    // Calculate integer coordinates based on division size
    // Use FloorToInt for consistent mapping
    Coordinates.X = (DivisionsX > 0 && DivisionSize > KINDA_SMALL_NUMBER) ? FMath::FloorToInt(GridSpaceLocation.X / DivisionSize) : 0;
    Coordinates.Y = (DivisionsY > 0 && DivisionSize > KINDA_SMALL_NUMBER) ? FMath::FloorToInt(GridSpaceLocation.Y / DivisionSize) : 0;
    Coordinates.Z = (DivisionsZ > 0 && DivisionSize > KINDA_SMALL_NUMBER) ? FMath::FloorToInt(GridSpaceLocation.Z / DivisionSize) : 0;

    // Clamp to ensure coordinates are within [0, Divisions-1]
    ClampCoordinates(Coordinates); // Use const version (modifies parameter)
    return Coordinates;
}

FVector ANavigationVolume3D::ConvertCoordinatesToLocation(const FIntVector& Coordinates) const
{
    FIntVector ClampedCoordinates = Coordinates;
    ClampCoordinates(ClampedCoordinates); // Ensure coordinates are valid before calculating location

    // Calculate world location at the center of the cell
    FVector GridSpaceLocation;
    GridSpaceLocation.X = (ClampedCoordinates.X + 0.5f) * DivisionSize;
    GridSpaceLocation.Y = (ClampedCoordinates.Y + 0.5f) * DivisionSize;
    GridSpaceLocation.Z = (ClampedCoordinates.Z + 0.5f) * DivisionSize;

    return UKismetMathLibrary::TransformLocation(GetActorTransform(), GridSpaceLocation);
}

bool ANavigationVolume3D::AreCoordinatesValid(const FIntVector& Coordinates) const
{
    return Coordinates.X >= 0 && Coordinates.X < DivisionsX &&
           Coordinates.Y >= 0 && Coordinates.Y < DivisionsY &&
           Coordinates.Z >= 0 && Coordinates.Z < DivisionsZ;
}

void ANavigationVolume3D::ClampCoordinates(FIntVector& Coordinates) const
{
    Coordinates.X = FMath::Clamp(Coordinates.X, 0, DivisionsX - 1);
    Coordinates.Y = FMath::Clamp(Coordinates.Y, 0, DivisionsY - 1);
    Coordinates.Z = FMath::Clamp(Coordinates.Z, 0, DivisionsZ - 1);
}

// --- Helper for Grid Visualization ---

void ANavigationVolume3D::CreateLine(const FVector& Start, const FVector& End, const FVector& Normal, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    const float HalfLineThickness = LineThickness * 0.5f;
    FVector LineDirection = End - Start;
    if (!LineDirection.Normalize()) return; // Skip zero-length lines

    // Calculate perpendicular directions for thickness
    FVector ThicknessDir1 = FVector::CrossProduct(LineDirection, Normal).GetSafeNormal();
    if (ThicknessDir1.IsNearlyZero()) // Handle case where Normal is parallel to LineDirection
    {
         LineDirection.FindBestAxisVectors(ThicknessDir1, ThicknessDir1); // Find arbitrary perpendicular
         ThicknessDir1 = FVector::CrossProduct(LineDirection, ThicknessDir1).GetSafeNormal();
    }
    FVector ThicknessDir2 = FVector::CrossProduct(LineDirection, ThicknessDir1).GetSafeNormal();

    // Lambda to generate vertices and triangles for one flat quad
    auto CreateQuad = [&](const FVector& ThicknessDirection)
    {
        int32 VtxStartIdx = Vertices.Num();
        Vertices.Add(Start + ThicknessDirection * HalfLineThickness); // 0
        Vertices.Add(End   + ThicknessDirection * HalfLineThickness); // 1
        Vertices.Add(Start - ThicknessDirection * HalfLineThickness); // 2
        Vertices.Add(End   - ThicknessDirection * HalfLineThickness); // 3

        // Triangle 1 (0-1-2) - Counter-clockwise order
        Triangles.Add(VtxStartIdx + 0);
        Triangles.Add(VtxStartIdx + 1);
        Triangles.Add(VtxStartIdx + 2);
        // Triangle 2 (2-1-3)
        Triangles.Add(VtxStartIdx + 2);
        Triangles.Add(VtxStartIdx + 1);
        Triangles.Add(VtxStartIdx + 3);
    };

    CreateQuad(ThicknessDir1);
    CreateQuad(ThicknessDir2);
}

// --- Node Accessor ---

NavNode* ANavigationVolume3D::GetNode(FIntVector Coordinates) // Not const
{
    // Clamp coordinates first to attempt to bring potentially invalid input into range
    ClampCoordinates(Coordinates); // Use const version (modifies parameter)

    // Check for invalid grid dimensions before index calculation
    if (DivisionsX <= 0 || DivisionsY <= 0 || DivisionsZ <= 0) {
        // Error logged in BeginPlay if it happens there. Avoid redundant log here.
        return nullptr;
    }

    // Calculate 1D index
    const int32 DivisionPerLevel = DivisionsX * DivisionsY;
    const int32 Index = (Coordinates.Z * DivisionPerLevel) + (Coordinates.Y * DivisionsX) + Coordinates.X;

    // Check if calculated index is valid for the Nodes array
    if (Nodes.IsValidIndex(Index)) {
        return &Nodes[Index]; // Return pointer to the node
    }
    else {
        // This indicates a serious logic error if coordinates were clamped but index is still bad
        UE_LOG(LogTemp, Error, TEXT("ANavigationVolume3D (%s): GetNode calculated invalid index %d for Nodes array (Size: %d) even after clamping Coords: %s"), *GetName(), Index, Nodes.Num(), *Coordinates.ToString());
        return nullptr;
    }
}