#include "FProgressTimer.h"

FProgressTimer::FProgressTimer(const float InDuration) :
	Duration(InDuration), Progress(ZeroCompletion), bIsPaused(false) {}

void FProgressTimer::IncreaseProgress(const float DeltaTime)
{
	if (Progress >= FullCompletion || bIsPaused)
	{
		return;
	}
	UpdateProgress(Progress += DeltaTime / Duration);
}

void FProgressTimer::DecreaseProgress(const float DeltaTime)
{
	if (Progress <= ZeroCompletion || bIsPaused)
	{
		return;
	}
	UpdateProgress(Progress -= DeltaTime / Duration);
}

void FProgressTimer::Reset()
{
	Progress = 0.f;
}

void FProgressTimer::UpdateProgress(const float NewProgress)
{
	Progress = FMath::Clamp(NewProgress, ZeroCompletion, FullCompletion);
}
