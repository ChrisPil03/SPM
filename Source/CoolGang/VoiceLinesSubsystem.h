#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoiceLinesSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnVoiceLineBegin)

UCLASS()
class COOLGANG_API UVoiceLinesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void EnqueueVoiceLine(USoundBase* VoiceLine);

	template <typename T>
	void AddOnVoiceLineBeginFunction(T* Object, void (T::*Func)())
	{
		OnVoiceLineBegin.AddUObject(Object, Func);
	}
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	void PlayNextInQueue();
	void OnVoiceLineFinished();

	UPROPERTY()
	UAudioComponent* AudioComponent;
	
	TQueue<USoundBase*> VoiceLineQueue;
	bool bIsVoiceLinePlaying;
	FTimerHandle VoiceLineTimerHandle;
	FOnVoiceLineBegin OnVoiceLineBegin;
};
