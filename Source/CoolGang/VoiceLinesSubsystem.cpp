#include "VoiceLinesSubsystem.h"
#include "Components/AudioComponent.h"

void UVoiceLinesSubsystem::EnqueueVoiceLine(USoundBase* VoiceLine)
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

void UVoiceLinesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	bIsVoiceLinePlaying = false;
	
	if (GEngine && GEngine->GameViewport->GetWorld())
	{
		if (UWorld* World = GEngine->GameViewport->GetWorld())
		{
			AudioComponent = NewObject<UAudioComponent>(World);
			AudioComponent->RegisterComponentWithWorld(World);
		}
	}
}

void UVoiceLinesSubsystem::PlayNextInQueue()
{
	if (!VoiceLineQueue.IsEmpty())
	{
		USoundBase* NextVoiceLine = nullptr;
		VoiceLineQueue.Dequeue(NextVoiceLine);

		if (NextVoiceLine)
		{
			bIsVoiceLinePlaying = true;
			AudioComponent->SetSound(NextVoiceLine);
			AudioComponent->Play();

			if (OnVoiceLineBegin.IsBound())
			{
				OnVoiceLineBegin.Broadcast();
			}
			
			float Duration = NextVoiceLine->GetDuration();
			if (Duration > 0.f)
			{
				GetWorld()->GetTimerManager().SetTimer(
					VoiceLineTimerHandle, 
					this, 
					&UVoiceLinesSubsystem::OnVoiceLineFinished, 
					Duration, 
					false
				);
			}
		}
	}
}

void UVoiceLinesSubsystem::OnVoiceLineFinished()
{
	bIsVoiceLinePlaying = false;
	PlayNextInQueue();
}
