// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DisplayTextMessageSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDisplayMessage, FString, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRemoveMessage);

UCLASS(BlueprintType)
class COOLGANG_API UDisplayTextMessageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void DisplayMessage(const FString& Message) const;
	void DisplayMessageForSeconds(const FString& Message, float Seconds);
	void RemoveCurrentMessage() const;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDisplayMessage DisplayMessageDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FRemoveMessage RemoveMessageDelegate;
	
	FString LastMessage;
	FTimerHandle MessageTimerHandle;
};
