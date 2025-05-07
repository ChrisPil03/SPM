#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScoreManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

UENUM(BlueprintType)
enum class EScoreType : uint8
{
	SpiderKill UMETA(DisplayName = "Spider Kill"),
	WaspKill UMETA(DisplayName = "Wasp Kill"),
	ObjectiveDownloadCompleted UMETA(DisplayName = "Objective Download Completed"),
	ObjectiveButtonsCompleted UMETA(DisplayName = "Objective Buttons Completed"),
	ObjectiveServersCompleted UMETA(DisplayName = "Objective Servers Completed"),
	ObjectiveGeneratorCompleted UMETA(DisplayName = "Objective Generator Completed")
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOLGANG_API UScoreManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UScoreManagerComponent();

protected:
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(EScoreType ScoreType);

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetTotalScore() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetScoreValue(EScoreType ScoreType) const;
	
	UPROPERTY(BlueprintAssignable, Category = "ScoreEvents")
	FOnScoreChanged OnScoreChanged;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Score")
	int32 TotalScore = 0;

	UPROPERTY(VisibleAnywhere, Category = "Score")
	float BaseScoreMultiplier = 1.f;
	
	UPROPERTY(VisibleAnywhere, Category = "Score")
	float CurrentScoreMultiplier = 1.f;
};

USTRUCT(BlueprintType)
struct FScoreEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EScoreType ScoreType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ScoreValue;
};

UCLASS(BlueprintType)
class UScoreDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FScoreEntry> ScoreEntries;

	const TMap<EScoreType, int32>& GetScoreMap();

private:
	TMap<EScoreType, int32> CachedMap;
	bool bBuilt = false;
};