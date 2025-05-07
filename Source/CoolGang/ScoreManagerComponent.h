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
	
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(const EScoreType ScoreType);

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetTotalScore() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetScoreByType(const EScoreType ScoreType);
	
	UPROPERTY(BlueprintAssignable, Category = "Score")
	FOnScoreChanged OnScoreChanged;
	
private:
	int32 GetScoreValue(const EScoreType ScoreType) const;
	
	UPROPERTY(VisibleAnywhere, Category = "Score")
	int32 TotalScore;

	UPROPERTY(VisibleAnywhere, Category = "Score")
	float BaseScoreMultiplier;
	
	UPROPERTY(VisibleAnywhere, Category = "Score")
	float CurrentScoreMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "ScoreAmounts")
	int32 SpiderKillScore;

	UPROPERTY(EditDefaultsOnly, Category = "ScoreAmounts")
	int32 WaspKillScore;

	UPROPERTY(EditDefaultsOnly, Category = "ScoreAmounts")
	int32 ObjectiveDownloadScore;

	UPROPERTY(EditDefaultsOnly, Category = "ScoreAmounts")
	int32 ObjectiveButtonsScore;

	UPROPERTY(EditDefaultsOnly, Category = "ScoreAmounts")
	int32 ObjectiveServersScore;

	UPROPERTY(EditDefaultsOnly, Category = "ScoreAmounts")
	int32 ObjectiveGeneratorScore;

	TMap<EScoreType, int32> ScoreByTypeMap;
};