#include "ScoreManagerComponent.h"

UScoreManagerComponent::UScoreManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UScoreManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UScoreManagerComponent::AddScore(EScoreType ScoreType)
{
}

int32 UScoreManagerComponent::GetTotalScore() const
{
	return 0;
}

int32 UScoreManagerComponent::GetScoreValue(EScoreType ScoreType) const
{
	return 0;
}

