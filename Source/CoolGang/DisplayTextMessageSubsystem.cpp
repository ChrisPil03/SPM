#include "DisplayTextMessageSubsystem.h"

void UDisplayTextMessageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDisplayTextMessageSubsystem::DisplayMessage(const FString& Message) const
{
	// if (DisplayMessageDelegate.IsBound())
	// {
	// 	DisplayMessageDelegate.Broadcast(Message);
	// }
}

void UDisplayTextMessageSubsystem::DisplayMessageForSeconds(const FString& Message, float Seconds)
{
	DisplayMessage(Message);
	GetWorld()->GetTimerManager().SetTimer(
		MessageTimerHandle,
		this,
		&UDisplayTextMessageSubsystem::RemoveCurrentMessage,
		Seconds,
		false);
}

void UDisplayTextMessageSubsystem::RemoveCurrentMessage() const
{
	if (RemoveMessageDelegate.IsBound())
	{
		RemoveMessageDelegate.Broadcast();
	}
}
