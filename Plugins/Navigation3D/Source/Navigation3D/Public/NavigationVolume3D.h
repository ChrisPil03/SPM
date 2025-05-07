// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavNode.h" // *** Include the FULL definition of NavNode ***
#include "NavigationVolume3D.generated.h" // *** MUST BE LAST INCLUDE ***

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
    ENVR_VolumeNotInitialized    UMETA(DisplayName = "Navigation Volume Not Initialized"),
    ENVR_PathToSelf              UMETA(DisplayName = "Path to self"),
    ENVR_UnknownError            UMETA(DisplayName = "Unknown Error")
    
};

UCLASS()
class NAVIGATION3D_API ANavigationVolume3D : public AActor // Replace NAVIGATION3D_API if your module API is different
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ANavigationVolume3D();

protected: // Changed visibility for components to protected for easier subclass access if needed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Display", meta=(AllowPrivateAccess="true"))
    bool bDrawGridLinesInEditor = true;

    // The default (root) scene component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> DefaultSceneComponent; // Use TObjectPtr for UE5+

    // The procedural mesh responsbile for rendering the grid visualization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UProceduralMeshComponent> ProceduralMesh; // Use TObjectPtr for UE5+

    // The number of divisions in the grid along the X axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Dimensions", meta = (AllowPrivateAccess = "true", ClampMin = 1, UIMin = 1))
    int32 DivisionsX = 10;

    // The number of divisions in the grid along the Y axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Dimensions", meta = (AllowPrivateAccess = "true", ClampMin = 1, UIMin = 1))
    int32 DivisionsY = 10;

    // The number of divisions in the grid along the Z axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Dimensions", meta = (AllowPrivateAccess = "true", ClampMin = 1, UIMin = 1))
    int32 DivisionsZ = 10;

    // The size (in world units) of each grid division cube
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Dimensions", meta = (AllowPrivateAccess = "true", ClampMin = 1.0, UIMin = 1.0))
    float DivisionSize = 100.0f;

    // The minimum number of shared axes a neighbor must have to be considered connected (1 = faces, 2 = edges/corners). 0 allows corners only.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connectivity", meta = (AllowPrivateAccess = "true", ClampMin = 0, ClampMax = 2, UIMin = 0, UIMax = 2))
    int32 MinSharedNeighborAxes = 1; // Changed default to 1 (face connectivity is common)

    // The thickness of the grid lines rendered in the editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display", meta = (AllowPrivateAccess = "true", ClampMin = 0, UIMin = 0.0))
    float LineThickness = 2.0f;

    // The color and opacity of the grid lines rendered in the editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display", meta = (AllowPrivateAccess = "true"))
    FLinearColor Color = FLinearColor(0.0f, 0.7f, 0.0f, 0.5f); // Changed default color

    // Material used to render the grid visualization in the editor. Assign M_Grid here.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMaterialInterface> GridMaterial;

    // Object types to consider as obstacles during pathfinding checks (precomputed in BeginPlay)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<TEnumAsByte<EObjectTypeQuery>> ObstacleObjectTypes;

    // Specific actor class to filter for obstacles (can be None/nullptr)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TSubclassOf<AActor> ObstacleActorClassFilter;


public:
    /**
    * Called when an instance of this class is placed (in editor) or spawned.
    * Rebuilds the visual representation of the grid.
    * @param Transform The transform the actor was constructed at.
    */
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Called every frame (if PrimaryActorTick.bCanEverTick is true) */
    virtual void Tick(float DeltaTime) override;

    /**
    * Finds a path from the StartLocation to the DestinationLocation using A*.
    * Uses pre-calculated obstacle information from BeginPlay.
    * @param StartLocation World location to start the path from.
    * @param DestinationLocation World location to end the path at.
    * @param OutPath Output array filled with world locations representing the path steps (including start and end if found). Cleared if path not found.
    * @return True if a path was successfully found, false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "NavigationVolume3D", meta = (DisplayName = "Find Path"))
    ENavigationVolumeResult FindPath(const FVector& StartLocation, const FVector& DestinationLocation, TArray<FVector>& OutPath) const; // Mark const

    /**
    * Converts a world space location to grid coordinates. Clamped to volume bounds.
    * @param Location The world location to convert.
    * @return The corresponding grid coordinates (FIntVector).
    */
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D") // Changed to BlueprintPure
    FIntVector ConvertLocationToCoordinates(const FVector& Location) const; // Mark const

    /**
    * Converts grid coordinates to the world space location at the center of that grid cell. Clamped to volume bounds.
    * @param Coordinates The grid coordinates to convert.
    * @return The world space location (FVector).
    */
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D") // Changed to BlueprintPure
    FVector ConvertCoordinatesToLocation(const FIntVector& Coordinates) const; // Mark const

    //~ Getters for grid properties (inline for performance)
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    FORCEINLINE int32 GetTotalDivisions() const { return DivisionsX * DivisionsY * DivisionsZ; }
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    FORCEINLINE int32 GetDivisionsX() const { return DivisionsX; }
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    FORCEINLINE int32 GetDivisionsY() const { return DivisionsY; }
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    FORCEINLINE int32 GetDivisionsZ() const { return DivisionsZ; }
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    FORCEINLINE float GetDivisionSize() const { return DivisionSize; }
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    inline float GetGridSizeX() const { return DivisionsX * DivisionSize; }
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    inline float GetGridSizeY() const { return DivisionsY * DivisionSize; }
    UFUNCTION(BlueprintPure, Category = "NavigationVolume3D|Grid Dimensions")
    inline float GetGridSizeZ() const { return DivisionsZ * DivisionSize; }


protected:
    /** Called when the game starts or when spawned. Initializes nodes and precomputes traversability. */
    virtual void BeginPlay() override;

    /** Called when this actor is being removed from a level */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


private:
    /** Internal storage for all navigation nodes */
    TArray<NavNode> Nodes;

    /**
    * Gets a pointer to the node at the specified coordinates. Performs bounds checking.
    * Returns nullptr if coordinates are invalid or index calculation fails.
    * NOTE: This is non-const because it's used internally by BeginPlay to initialize nodes.
    */
    NavNode* GetNode(FIntVector Coordinates); // *** NOT CONST ***

    /** Helper function to create the geometry for a single line segment for the grid visualization. */
    void CreateLine(const FVector& Start, const FVector& End, const FVector& Normal, TArray<FVector>& Vertices, TArray<int32>& Triangles);

    /** Checks if the given coordinates are within the valid grid range [0, Divisions-1]. */
    bool AreCoordinatesValid(const FIntVector& Coordinates) const; // Mark const

    /** Clamps the Coordinates parameter to be within the valid grid range [0, Divisions-1]. */
    void ClampCoordinates(FIntVector& Coordinates) const; // Mark const (modifies parameter, not object state)

    /** Iterates through all nodes and performs BoxOverlap checks to determine traversability based on Obstacle properties. */
    void PrecomputeNodeTraversability();
};