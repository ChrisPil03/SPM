#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoiceLineSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnVoiceLineBegin)

USTRUCT()
struct FVoiceLineEntry
{
	GENERATED_BODY()

	UPROPERTY()
	USoundBase* VoiceLine = nullptr;
	UPROPERTY()
	int32 Priority = 0;
};

UCLASS()
class COOLGANG_API UVoiceLineSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Priority 0 is the highest priority
	// Higher numbers equals lower priority
	// Priority under 0 will only play if no other voice line is currently playing.
	// Priority under 0 will be overrided if another voice line is enqueued with priority greater than 0
	void EnqueueVoiceLine(USoundBase* VoiceLine, const int32 Priority);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	void OnWorldInitialized(const UWorld::FActorsInitializedParams& Params);
	void PlayNextInQueue();
	void OnVoiceLineFinished();

	UPROPERTY()
	UAudioComponent* AudioComponent;
	
	TArray<FVoiceLineEntry> VoiceLineQueue;
	FVoiceLineEntry CurrentVoiceLine;
	bool bIsVoiceLinePlaying;
	FTimerHandle VoiceLineTimerHandle;
	FOnVoiceLineBegin OnVoiceLineBegin;
};
