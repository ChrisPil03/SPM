#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemySpawnConfigurationDataAsset.generated.h"

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
class COOLGANG_API UEnemySpawnConfigurationDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Configurations", meta = (TitleProperty = "EnemyClass"))
	TArray<FEnemyTypeSpawnConfig> EnemyConfigs;
	
	UEnemySpawnConfigurationDataAsset() {}
};