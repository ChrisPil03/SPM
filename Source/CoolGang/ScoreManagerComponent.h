#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScoreManagerComponent.generated.h"

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreChangedWithType, EScoreType, ScoreType, int32, NewScore);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRequestAddScore, EScoreType);
inline FOnRequestAddScore OnRequestAddScore;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOLGANG_API UScoreManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UScoreManagerComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(const EScoreType ScoreType, const int32 Score);

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetTotalScore() const;

	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScoreByType(const EScoreType ScoreType);

	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScoreValue(const EScoreType ScoreType) const;

	UFUNCTION(BlueprintPure, Category = "Score")
	float GetScoreMultiplier() const { return CurrentScoreMultiplier; }
	
	UPROPERTY(BlueprintAssignable, Category = "Score")
	FOnScoreChanged OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "Score")
	FOnScoreChangedWithType OnScoreChangedWithType; // Sends ScoreType + NewScore

protected:
	virtual void BeginPlay() override;
	
private:
	void HandleAddScore(const EScoreType ScoreType);
	void BindOnMinutePassed();
	
	UFUNCTION()
	void IncreaseMultiplierPerMinute();
	
	UPROPERTY(VisibleAnywhere, Category = "Score")
	int32 TotalScore;

	UPROPERTY(VisibleAnywhere, Category = "Score")
	float BaseScoreMultiplier;
	
	UPROPERTY(VisibleAnywhere, Category = "Score")
	float CurrentScoreMultiplier;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Score")
	float MinuteMultiplierIncrease;

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