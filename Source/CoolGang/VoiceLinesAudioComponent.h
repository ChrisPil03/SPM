// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "VoiceLinesAudioComponent.generated.h"

UCLASS()
class COOLGANG_API UVoiceLinesAudioComponent : public UAudioComponent
{
	GENERATED_BODY()
public:
	UVoiceLinesAudioComponent();
	void EnqueueVoiceLine(USoundBase* VoiceLine);
	
private:
	void PlayNextInQueue();
	void OnVoiceLineFinished();
	
	TQueue<USoundBase*> VoiceLineQueue;
	bool bIsVoiceLinePlaying;
	FTimerHandle VoiceLineTimerHandle;
};
