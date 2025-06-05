#include "ScoreManagerComponent.h"

#include "DiveGameMode.h"
#include "Kismet/GameplayStatics.h"

UScoreManagerComponent::UScoreManagerComponent() :
	TotalScore(0),
	BaseScoreMultiplier(1.f),
	CurrentScoreMultiplier(1.f),
	MinuteMultiplierIncrease(0.1f),
	SpiderKillScore(10),
	WaspKillScore(10),
	ObjectiveDownloadScore(10),
	ObjectiveButtonsScore(10),
	ObjectiveServersScore(10),
	ObjectiveGeneratorScore(10)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScoreManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	BindOnMinutePassed();
	OnRequestAddScore.AddUObject(this, &UScoreManagerComponent::HandleAddScore);
}

void UScoreManagerComponent::AddScore(const EScoreType ScoreType, const int32 Score)
{
	int32 ScoreToAdd = Score * CurrentScoreMultiplier;
	TotalScore += ScoreToAdd;
	ScoreByTypeMap.FindOrAdd(ScoreType) += ScoreToAdd;

	if (OnScoreChanged.IsBound())
	{
		OnScoreChanged.Broadcast(TotalScore);
	}
	OnScoreChangedWithType.Broadcast(ScoreType, TotalScore); // <-- NEW

}

int32 UScoreManagerComponent::GetTotalScore() const
{
	return TotalScore;
}

int32 UScoreManagerComponent::GetScoreByType(const EScoreType ScoreType)
{
	if (ScoreByTypeMap.Contains(ScoreType))
	{
		return *ScoreByTypeMap.Find(ScoreType);
	}
	return 0;
}

int32 UScoreManagerComponent::GetScoreValue(const EScoreType ScoreType) const
{
	switch (ScoreType) {
	case EScoreType::SpiderKill: return SpiderKillScore;
	case EScoreType::WaspKill: return WaspKillScore;
	case EScoreType::ObjectiveDownloadCompleted: return ObjectiveDownloadScore;
	case EScoreType::ObjectiveButtonsCompleted: return ObjectiveButtonsScore;
	case EScoreType::ObjectiveServersCompleted: return ObjectiveServersScore;
	case EScoreType::ObjectiveGeneratorCompleted: return ObjectiveGeneratorScore;
	default: return 0;
	}
}

void UScoreManagerComponent::HandleAddScore(const EScoreType ScoreType)
{
	AddScore(ScoreType, GetScoreValue(ScoreType));
}

void UScoreManagerComponent::BindOnMinutePassed()
{
	if (ADiveGameMode* GameMode = Cast<ADiveGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->AddOnMinutePassedFunction(this, &UScoreManagerComponent::IncreaseMultiplierPerMinute);
	}
}

void UScoreManagerComponent::IncreaseMultiplierPerMinute()
{
	BaseScoreMultiplier += MinuteMultiplierIncrease;
	CurrentScoreMultiplier += MinuteMultiplierIncrease;
}