#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavNode.h"
#include <atomic> // For std::atomic
#include "NavigationVolume3D.generated.h" // MUST BE LAST INCLUDE

// Standard library includes


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
    ENVR_VolumeNotReady          UMETA(DisplayName = "Navigation Volume Not Ready"),
    ENVR_RequestingActorInvalid  UMETA(DisplayName = "Requesting Actor Invalid"),
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

// Delegate for async pathfinding completion
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPathfindingComplete, ENavigationVolumeResult, Result, const TArray<FVector>&, Path);

UCLASS()
class NAVIGATION3D_API ANavigationVolume3D : public AActor
{
    GENERATED_BODY()

public:
    ANavigationVolume3D();

// Existing UPROPERTY declarations (Divisions, Size, Material, etc.)
// ... (Ensure all your existing UPROPERTIES are here) ...
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug", meta=(EditCondition="bDrawPathfindingDebug"))
    bool bPauseOnLongPath = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug", meta=(EditCondition="bDrawPathfindingDebug && bPauseOnLongPath", ClampMin="1", UIMin="1"))
    int32 LongPathThreshold = 80;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding|Debug", meta=(EditCondition="bDrawPathfindingDebug"))
    bool bOnlyDrawDebugForLongPaths = false;

public:
    UFUNCTION(BlueprintCallable, Category = "NavigationVolume3D", meta = (DisplayName = "Find Random Valid Location In Radius"))
    bool FindRandomValidLocationInRadius(
        const FVector& Origin,
        float WorldRadius,
        FVector& OutValidLocation,
        const AActor* ActorToIgnoreForLOS = nullptr,
        int32 MaxCandidatesToCollect = 30
    ) const;
    
    virtual void OnConstruction(const FTransform& Transform) override;

    // New async pathfinding function
    UFUNCTION(BlueprintCallable, Category = "NavigationVolume3D", meta = (DisplayName = "Find Path Async"))
    void FindPathAsync(
        const AActor* RequestingActor,
        const FVector& StartLocation,
        const FVector& DestinationLocation,
        FOnPathfindingComplete OnCompleteCallback
    );

    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D")
    FIntVector ConvertLocationToCoordinates(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D")
    FVector ConvertCoordinatesToLocation(const FIntVector& Coordinates) const;

    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    FORCEINLINE int32 GetTotalDivisions() const { return DivisionsX * DivisionsY * DivisionsZ; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    TArray<NavNode> Nodes;
    bool bNodesInitializedAndFinalized = false;
    
    // Atomic counter for unique search IDs
    std::atomic<uint32_t> AtomicPathfindingSearchIDCounter{0};

    // Internal struct to bundle results from the async task
    struct FPathfindingInternalResultBundle
    {
        ENavigationVolumeResult ResultCode = ENavigationVolumeResult::ENVR_UnknownError;
        TArray<FVector> PathPoints;
        TArray<FDebugSphereData> DebugSpheresToDraw_TaskLocal;
        TArray<FDebugLineData> DebugLinesToDraw_TaskLocal;
        bool bShouldDrawThisPathAndExploration_TaskLocal = false;
        bool bIsLongPath_TaskLocal = false;
        FString ActorNameForLog; // For logging if actor becomes invalid

        FPathfindingInternalResultBundle(const FString& InActorName = TEXT("UnknownActor")) : ActorNameForLog(InActorName) {}
    };

    // The core pathfinding logic, designed to be run off-thread
    FPathfindingInternalResultBundle ExecutePathfindingOnThread(
        TWeakObjectPtr<ANavigationVolume3D> WeakThis, // Pass weak ptr to self
        TWeakObjectPtr<const AActor> WeakRequestingActor,
        const FVector& StartLocation,
        const FVector& DestinationLocation,
        uint32_t PathSearchID // The unique ID for this specific search
    );
    
    // Modified debug helpers to populate local arrays for the task
    void AddDebugSphere_TaskLocal(TArray<FDebugSphereData>& DebugSpheresArray, const FVector& Center, float Radius, const FColor& InSphereColor, int32 Segments = 12) const;
    void AddDebugLine_TaskLocal(TArray<FDebugLineData>& DebugLinesArray, const FVector& Start, const FVector& End, const FColor& InLineColor, float Thickness = 1.f) const;
    
    // Actual drawing function, called on game thread
    void FlushCollectedDebugDraws(UWorld* World, float Lifetime, const TArray<FDebugSphereData>& Spheres, const TArray<FDebugLineData>& Lines, bool bIsLongPathContext = false) const;

    NavNode* GetNode(FIntVector Coordinates); 
    const NavNode* GetConstNode(FIntVector Coordinates) const;

    void CreateLine(const FVector& Start, const FVector& End, const FVector& Normal, TArray<FVector>& Vertices, TArray<int32>& Triangles);
    bool AreCoordinatesValid(const FIntVector& Coordinates) const;
    void ClampCoordinates(FIntVector& Coordinates) const;
    void PrecomputeNodeTraversability();
};