// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPadTriggerBoxComponent.h"

#include "GameFramework/Character.h"


void UJumpPadTriggerBoxComponent::BeginPlay()
{
	Super::BeginPlay();
	OnComponentBeginOverlap.AddDynamic(this, &UJumpPadTriggerBoxComponent::OnBoxBeginOverlap);
}

void UJumpPadTriggerBoxComponent::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Player = Cast<ACharacter>(OtherActor);
	if (Player)
	{
		FVector LaunchVelocity = FVector(0.f, 0.f, ZForce);
		Player->LaunchCharacter(LaunchVelocity, false, true);
	}
}
