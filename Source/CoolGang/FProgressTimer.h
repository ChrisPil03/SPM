#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE(FTimerCompletionDelegate)

class COOLGANG_API FProgressTimer
{
public:
	static constexpr float FullCompletion = 1.f;
	static constexpr float ZeroCompletion = 0.f;
	
	FProgressTimer(const float InDuration);
	~FProgressTimer() {}
	
	void IncreaseProgress(const float DeltaTime);
	void DecreaseProgress(const float DeltaTime);
	
	void Reset();

	void SetIsPaused(const bool bNewState) { bIsPaused = bNewState; }
	bool GetIsPaused() const { return bIsPaused; }
	float GetProgress() const { return Progress; }

	void SetCompletionDelegate(const FTimerCompletionDelegate& NewDelegate) { CompletionDelegate = NewDelegate; }

private:
	void UpdateProgress(const float NewProgress);
	
	float Duration;
	float Progress;
	bool bIsPaused;

	FTimerCompletionDelegate CompletionDelegate;
};
