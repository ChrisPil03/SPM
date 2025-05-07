#include "ScoreManagerComponent.h"

UScoreManagerComponent::UScoreManagerComponent() :
	TotalScore(0),
	BaseScoreMultiplier(1.f),
	CurrentScoreMultiplier(1.f),
	SpiderKillScore(10),
	WaspKillScore(10),
	ObjectiveDownloadScore(10),
	ObjectiveButtonsScore(10),
	ObjectiveServersScore(10),
	ObjectiveGeneratorScore(10)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScoreManagerComponent::AddScore(const EScoreType ScoreType)
{
	const int32 Score = GetScoreValue(ScoreType);
	TotalScore += Score;
	ScoreByTypeMap.FindOrAdd(ScoreType) += Score;
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
