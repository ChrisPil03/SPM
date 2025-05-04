#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE(FTimerCompletionDelegate)

class COOLGANG_API FProgressTimer
{
public:
	static constexpr float FullCompletion = 1.f;
	static constexpr float ZeroCompletion = 0.f;
	
	FProgressTimer(const float InDuration);
	
	void IncreaseProgress(const float DeltaTime);
	void DecreaseProgress(const float DeltaTime);
	
	void ResetTimer();

	void SetProgress(const float NewProgress) { UpdateProgress(NewProgress); }
	void SetIsPaused(const bool bNewState) { bIsPaused = bNewState; }
	bool GetIsPaused() const { return bIsPaused; }
	bool GetIsCompleted() const { return bIsCompleted; }
	float GetProgress() const { return Progress; }

	void SetCompletionDelegate(const FTimerCompletionDelegate& NewDelegate) { CompletionDelegate = NewDelegate; }

	template <typename T>
	void SetCompletionFunction(T* Object, void (T::*Func)())
	{
		CompletionDelegate.BindUObject(Object, Func);
	}

private:
	void UpdateProgress(const float NewProgress);
	
	float Duration;
	float Progress;
	bool bIsPaused;
	bool bIsCompleted;

	FTimerCompletionDelegate CompletionDelegate;
};
