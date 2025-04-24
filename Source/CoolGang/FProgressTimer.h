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

	void SetProgress(const float NewProgress) { UpdateProgress(NewProgress); }
	void SetIsPaused(const bool bNewState) { bIsPaused = bNewState; }
	bool GetIsPaused() const { return bIsPaused; }
	bool GetIsCompleted() const { return bIsCompleted; }
	float GetProgress() const { return Progress; }

	void SetCompletionDelegate(const FTimerCompletionDelegate& NewDelegate) { CompletionDelegate = NewDelegate; }

private:
	void UpdateProgress(const float NewProgress);
	
	float Duration;
	float Progress;
	bool bIsPaused;
	bool bIsCompleted;

	FTimerCompletionDelegate CompletionDelegate;
};
