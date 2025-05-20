#include "FProgressTimer.h"

FProgressTimer::FProgressTimer(const float InDuration) :
	Duration(InDuration), Progress(ZeroCompletion), ElapsedTime(0.f),
	bIsPaused(false), bIsCompleted(false) {}

void FProgressTimer::IncreaseProgress(const float DeltaTime)
{
	if (bIsPaused || bIsCompleted)
	{
		return;
	}

	ElapsedTime += DeltaTime;
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

void FProgressTimer::ResetTimer()
{
	Progress = ZeroCompletion;
	bIsCompleted = false;
	ElapsedTime = 0.f;
}

void FProgressTimer::GetElapsedMinutesAndSeconds(int32& OutMinutes, int32& OutSeconds) const
{
	OutMinutes = static_cast<int32>(ElapsedTime) / 60;
	OutSeconds = static_cast<int32>(ElapsedTime) % 60;
}

void FProgressTimer::UpdateProgress(const float NewProgress)
{
	Progress = FMath::Clamp(NewProgress, ZeroCompletion, FullCompletion);
}
