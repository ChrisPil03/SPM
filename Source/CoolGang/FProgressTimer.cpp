#include "FProgressTimer.h"

FProgressTimer::FProgressTimer(const float InDuration) :
	Duration(InDuration), Progress(ZeroCompletion),
	bIsPaused(false), bIsCompleted(false) {}

void FProgressTimer::IncreaseProgress(const float DeltaTime)
{
	if (bIsPaused || bIsCompleted)
	{
		return;
	}
	
	UpdateProgress(Progress += DeltaTime / Duration);
	
	if (Progress >= FullCompletion)
	{
		bIsCompleted = true;
		if (CompletionDelegate.IsBound())
		{
			CompletionDelegate.Execute();
		}
	}
}

void FProgressTimer::DecreaseProgress(const float DeltaTime)
{
	if (bIsCompleted)
	{
		bIsCompleted = false;
	}

	if (!bIsPaused && Progress > ZeroCompletion)
	{
		UpdateProgress(Progress -= DeltaTime / Duration);
	}
}

void FProgressTimer::Reset()
{
	Progress = ZeroCompletion;
	bIsCompleted = false;
}

void FProgressTimer::UpdateProgress(const float NewProgress)
{
	Progress = FMath::Clamp(NewProgress, ZeroCompletion, FullCompletion);
}
