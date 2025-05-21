#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemySpawnConfigurationDataAsset.generated.h"

// Forward declare your base AI class.
// Ensure the actual AEnemyAI class is defined in its own header (e.g., "AI/EnemyAI.h")
// and that header is included wherever you use these types more extensively.
class AEnemyAI;

USTRUCT(BlueprintType)
struct FEnemyTypeSpawnConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Config")
	TSubclassOf<AEnemyAI> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Config", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxSpawnCount = 1;
};

UCLASS(BlueprintType)
class COOLGANG_API UEnemySpawnConfigurationDataAsset : public UDataAsset // Using UDataAsset for simplicity
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Configurations", meta = (TitleProperty = "EnemyClass"))
	TArray<FEnemyTypeSpawnConfig> EnemyConfigs;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Rate", meta = (ClampMin = "0", UIMin = "0"))
	double BaselineSpawnInterval = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Rate", meta = (ClampMin = "0", UIMin = "0"))
	double MinimumSpawnInterval = .4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Rate", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxSpawnIntervalIncreaseCount = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Rate", meta = (ClampMin = "0", UIMin = "0"))
	double SpawnIntervalScale = 2.5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Rate", meta = (ClampMin = "0", UIMin = "0"))
	double SpawnAccelerationRate = 3.4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Rate", meta = (ClampMin = "0", UIMin = "0"))
	double SpawnIntervalIncreaseTimer = 20.0;
	
	UEnemySpawnConfigurationDataAsset() {}
};