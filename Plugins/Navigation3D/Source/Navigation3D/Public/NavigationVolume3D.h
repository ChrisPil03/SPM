#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavNode.h" // Ensure this is the updated NavNode.h
#include "NavigationVolume3D.generated.h" // MUST BE LAST INCLUDE

class UProceduralMeshComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class ENavigationVolumeResult : uint8
{
    ENVR_Success                 UMETA(DisplayName = "Success"),
    ENVR_StartNodeInvalid        UMETA(DisplayName = "Start Node Invalid (Out of Bounds)"),
    ENVR_StartNodeBlocked        UMETA(DisplayName = "Start Node Blocked"),
    ENVR_EndNodeInvalid          UMETA(DisplayName = "End Node Invalid (Out of Bounds)"),
    ENVR_EndNodeBlocked          UMETA(DisplayName = "End Node Blocked"),
    ENVR_NoPathExists            UMETA(DisplayName = "No Path Found Between Valid Nodes"),
    ENVR_PathToSelf              UMETA(DisplayName = "Path to self"),
    ENVR_UnknownError            UMETA(DisplayName = "Unknown Error")
};

USTRUCT()
struct FDebugLineData {
    GENERATED_BODY()
    FVector Start;
    FVector End;
    FColor Color;
    float Thickness;
};

USTRUCT()
struct FDebugSphereData {
    GENERATED_BODY()
    FVector Center;
    float Radius;
    FColor Color;
    int32 Segments;
};

UCLASS()
class NAVIGATION3D_API ANavigationVolume3D : public AActor
{
    GENERATED_BODY()

public:
    ANavigationVolume3D();

// ... (Your existing UPROPERTY declarations for Divisions, Size, Material, etc.) ...
// Keep these as they are:
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Display", meta=(AllowPrivateAccess="true"))
    bool bDrawGridLinesInEditor = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> DefaultSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UProceduralMeshComponent> ProceduralMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Dimensions", meta = (AllowPrivateAccess = "true", ClampMin = 1, UIMin = 1))
    int32 DivisionsX = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Dimensions", meta = (AllowPrivateAccess = "true", ClampMin = 1, UIMin = 1))
    int32 DivisionsY = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Dimensions", meta = (AllowPrivateAccess = "true", ClampMin = 1, UIMin = 1))
    int32 DivisionsZ = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Dimensions", meta = (AllowPrivateAccess = "true", ClampMin = 1.0, UIMin = 1.0))
    float DivisionSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connectivity", meta = (AllowPrivateAccess = "true", ClampMin = 0, ClampMax = 2, UIMin = 0, UIMax = 2))
    int32 MinSharedNeighborAxes = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display", meta = (AllowPrivateAccess = "true", ClampMin = 0, UIMin = 0.0))
    float LineThickness = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display", meta = (AllowPrivateAccess = "true"))
    FLinearColor Color = FLinearColor(0.0f, 0.7f, 0.0f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMaterialInterface> GridMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<TEnumAsByte<EObjectTypeQuery>> ObstacleObjectTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TSubclassOf<AActor> ObstacleActorClassFilter;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug")
    bool bDrawPathfindingDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug", meta=(EditCondition="bDrawPathfindingDebug"))
    float DebugDrawLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug", meta=(EditCondition="bDrawPathfindingDebug"))
    float DebugNodeSphereRadius = 15.0f;

    // New Properties for conditional debugging
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug", meta=(EditCondition="bDrawPathfindingDebug"))
    bool bPauseOnLongPath = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug", meta=(EditCondition="bDrawPathfindingDebug && bPauseOnLongPath", ClampMin="1", UIMin="1"))
    int32 LongPathThreshold = 80; // Path length to trigger pause and detailed drawing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug", meta=(EditCondition="bDrawPathfindingDebug"))
    bool bOnlyDrawDebugForLongPaths = false; // If true, only draws debug if path > LongPathThreshold (and bPauseOnLongPath is true)

    
    // ... (Your UFUNCTION declarations for FindRandomValidLocationInRadius, OnConstruction, Tick, Getters) ...
    // Keep these:
    UFUNCTION(BlueprintCallable, Category = "NavigationVolume3D", meta = (DisplayName = "Find Random Valid Location In Radius"))
    bool FindRandomValidLocationInRadius(
        const FVector& Origin,
        float WorldRadius,
        FVector& OutValidLocation,
        const AActor* ActorToIgnoreForLOS = nullptr,
        int32 MaxCandidatesToCollect = 30
    ) const;
    
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "NavigationVolume3D", meta = (DisplayName = "Find Path"))
    ENavigationVolumeResult FindPath(const AActor* Actor, const FVector& StartLocation, const FVector& DestinationLocation, TArray<FVector>& OutPath);

    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D")
    FIntVector ConvertLocationToCoordinates(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D")
    FVector ConvertCoordinatesToLocation(const FIntVector& Coordinates) const;

    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    FORCEINLINE int32 GetTotalDivisions() const { return DivisionsX * DivisionsY * DivisionsZ; }
    // ... other getters ...

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    TArray<NavNode> Nodes;

    UPROPERTY() // Keep alive if this actor persists across PIE sessions with debug drawing on
    TArray<FDebugSphereData> DebugSpheresToDraw;
    UPROPERTY()
    TArray<FDebugLineData> DebugLinesToDraw;

    void AddDebugSphere(const FVector& Center, float Radius, const FColor& InSphereColor, int32 Segments = 12);
    void AddDebugLine(const FVector& Start, const FVector& End, const FColor& InLineColor, float Thickness = 1.f);
    void FlushDebugDraws(UWorld* World, float Lifetime);
    
    bool bNodesInitializedAndFinalized;
    // --- Search ID for A* data in NavNodes ---
    uint32_t CurrentPathfindingSearchID = 0; // Initialize to 0

    // Making GetNode non-const only, as it returns a non-const pointer
    // to allow modification of NavNode members like FScore_Pathfinding during search.
    // If a truly const version is needed elsewhere, it should return const NavNode*
    // and not allow modification of pathfinding-specific data.
    NavNode* GetNode(FIntVector Coordinates); 
    
    // Const version for read-only access
    const NavNode* GetConstNode(FIntVector Coordinates) const;

    void CreateLine(const FVector& Start, const FVector& End, const FVector& Normal, TArray<FVector>& Vertices, TArray<int32>& Triangles);
    bool AreCoordinatesValid(const FIntVector& Coordinates) const;
    void ClampCoordinates(FIntVector& Coordinates) const;
    void PrecomputeNodeTraversability();
};