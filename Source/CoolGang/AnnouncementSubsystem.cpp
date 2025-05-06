#include "AnnouncementSubsystem.h"
#include "DisplayTextMessageSubsystem.h"
#include "Components/AudioComponent.h"

void UAnnouncementSubsystem::EnqueueVoiceLineWithMessage(USoundBase* VoiceLine, const FString& Message)
{
	if (!VoiceLine)
	{
		return;
	}
	FMessageEntry Entry;
	Entry.VoiceLine = VoiceLine;
	Entry.Message = Message;
	MessageQueue.Enqueue(Entry);

	if (!bIsVoiceLinePlaying)
	{
		PlayNextInQueue();
	}
}

void UAnnouncementSubsystem::EnqueueVoiceLine(USoundBase* VoiceLine)
{
	EnqueueVoiceLineWithMessage(VoiceLine, "");
}

void UAnnouncementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UAnnouncementSubsystem::OnWorldInitialized);
}

void UAnnouncementSubsystem::OnWorldInitialized(const UWorld::FActorsInitializedParams& Params)
{
	bIsVoiceLinePlaying = false;
	
	if (UWorld* World = GetWorld())
	{
		AudioComponent = NewObject<UAudioComponent>(World);
		AudioComponent->RegisterComponentWithWorld(World);
		DisplayTextMessageSubsystem = World->GetSubsystem<UDisplayTextMessageSubsystem>();
	}
}

void UAnnouncementSubsystem::PlayNextInQueue()
{
	if (!MessageQueue.IsEmpty())
	{
		FMessageEntry NextMessage;
		MessageQueue.Dequeue(NextMessage);

		if (NextMessage.VoiceLine && AudioComponent)
		{
			bIsVoiceLinePlaying = true;
			AudioComponent->SetSound(NextMessage.VoiceLine);
			AudioComponent->Play();

			if (DisplayTextMessageSubsystem)
			{
				DisplayTextMessageSubsystem->DisplayMessage(NextMessage.Message);	
			}
			
			float Duration = NextMessage.VoiceLine->GetDuration();
			if (Duration > 0.f)
			{
				GetWorld()->GetTimerManager().SetTimer(
					VoiceLineTimerHandle, 
					this, 
					&UAnnouncementSubsystem::OnVoiceLineFinished, 
					Duration, 
					false
				);
			}
		}
	}
}

void UAnnouncementSubsystem::OnVoiceLineFinished()
{
	bIsVoiceLinePlaying = false;
	if (DisplayTextMessageSubsystem)
	{
		DisplayTextMessageSubsystem->RemoveCurrentMessage();
	}
	PlayNextInQueue();
}