// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceLinesAudioComponent.h"

UVoiceLinesAudioComponent::UVoiceLinesAudioComponent() :
	bIsVoiceLinePlaying(false)
{
}

void UVoiceLinesAudioComponent::EnqueueVoiceLine(USoundBase* VoiceLine)
{
	if (!VoiceLine)
	{
		return;
	}
	VoiceLineQueue.Enqueue(VoiceLine);

	if (!bIsVoiceLinePlaying)
	{
		PlayNextInQueue();
	}
}

void UVoiceLinesAudioComponent::PlayNextInQueue()
{
	if (!VoiceLineQueue.IsEmpty())
	{
		USoundBase* NextVoiceLine = nullptr;
		VoiceLineQueue.Dequeue(NextVoiceLine);

		if (NextVoiceLine)
		{
			bIsVoiceLinePlaying = true;
			SetSound(NextVoiceLine);
			Play();
			
            float Duration = NextVoiceLine->GetDuration();
            if (Duration > 0.f)
            {
                GetWorld()->GetTimerManager().SetTimer(
                    VoiceLineTimerHandle, 
                    this, 
                    &UVoiceLinesAudioComponent::OnVoiceLineFinished, 
                    Duration, 
                    false
                );
            }
		}
	}
}

void UVoiceLinesAudioComponent::OnVoiceLineFinished()
{
	bIsVoiceLinePlaying = false;
	PlayNextInQueue();
}
