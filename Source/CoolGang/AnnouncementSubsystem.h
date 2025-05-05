#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AnnouncementSubsystem.generated.h"

class UDisplayTextMessageSubsystem;
DECLARE_MULTICAST_DELEGATE(FOnVoiceLineBegin)

USTRUCT()
struct FMessageEntry
{
	GENERATED_BODY()

	UPROPERTY()
	USoundBase* VoiceLine;
	UPROPERTY()
	FString Message;
};

UCLASS()
class COOLGANG_API UAnnouncementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void EnqueueVoiceLineWithMessage(USoundBase* VoiceLine, const FString& Message);
	void EnqueueVoiceLine(USoundBase* VoiceLine);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	void OnWorldInitialized(const UWorld::FActorsInitializedParams& Params);
	void PlayNextInQueue();
	void OnVoiceLineFinished();

	UPROPERTY()
	UAudioComponent* AudioComponent;

	UPROPERTY()
	UDisplayTextMessageSubsystem* DisplayTextMessageSubsystem;
	
	TQueue<FMessageEntry> MessageQueue;
	bool bIsVoiceLinePlaying;
	FTimerHandle VoiceLineTimerHandle;
	FOnVoiceLineBegin OnVoiceLineBegin;
};
