#include "VoiceLineSubsystem.h"
#include "Components/AudioComponent.h"

void UVoiceLineSubsystem::EnqueueVoiceLine(USoundBase* VoiceLine, const int32 Priority)
{
	if (!VoiceLine && Priority < 0 && bIsVoiceLinePlaying)
	{
		return;
	}
	FVoiceLineEntry NewEntry;
	NewEntry.VoiceLine = VoiceLine;
	NewEntry.Priority = Priority;
	
	int32 InsertIndex = 0;
	while (InsertIndex < VoiceLineQueue.Num() &&
		VoiceLineQueue[InsertIndex].Priority <= Priority)
	{
		++InsertIndex;
	}

	VoiceLineQueue.Insert(NewEntry, InsertIndex);

	if (!bIsVoiceLinePlaying)
	{
		PlayNextInQueue();
	}
	else if (bIsVoiceLinePlaying && (Priority < CurrentVoiceLine.Priority || CurrentVoiceLine.Priority < 0))
	{
		if (AudioComponent && AudioComponent->IsPlaying())
		{
			AudioComponent->FadeOut(0.5f, 0.f);
			GetWorld()->GetTimerManager().SetTimer(
				VoiceLineTimerHandle,
				this,
				&UVoiceLineSubsystem::PlayNextInQueue,
				0.5f,
				false);
		}
	}
}

void UVoiceLineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UVoiceLineSubsystem::OnWorldInitialized);
}

void UVoiceLineSubsystem::OnWorldInitialized(const UWorld::FActorsInitializedParams& Params)
{
	bIsVoiceLinePlaying = false;
	
	if (UWorld* World = GetWorld())
	{
		AudioComponent = NewObject<UAudioComponent>(World);
		AudioComponent->RegisterComponentWithWorld(World);
	}
}

void UVoiceLineSubsystem::PlayNextInQueue()
{
	if (!VoiceLineQueue.IsEmpty())
	{
		FVoiceLineEntry NextVoiceLine = VoiceLineQueue.Last();
		VoiceLineQueue.RemoveAt(VoiceLineQueue.Num() - 1);

		if (NextVoiceLine.VoiceLine && AudioComponent)
		{
			CurrentVoiceLine = NextVoiceLine;
			bIsVoiceLinePlaying = true;
			
			AudioComponent->SetSound(NextVoiceLine.VoiceLine);
			AudioComponent->Play();
			
			float Duration = NextVoiceLine.VoiceLine->GetDuration();
			if (Duration > 0.f)
			{
				GetWorld()->GetTimerManager().SetTimer(
					VoiceLineTimerHandle, 
					this, 
					&UVoiceLineSubsystem::OnVoiceLineFinished, 
					Duration, 
					false);
			}
		}
	}
}

void UVoiceLineSubsystem::OnVoiceLineFinished()
{
	bIsVoiceLinePlaying = false;
	PlayNextInQueue();
}