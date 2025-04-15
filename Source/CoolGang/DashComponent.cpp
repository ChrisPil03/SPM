// Fill out your copyright notice in the Description page of Project Settings.


#include "DashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UDashComponent::UDashComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	
}


void UDashComponent::Dash()
{
	if (!OwnerCharacter) return;
	
	UE_LOG(LogTemp, Warning, TEXT("Dash"));
	FVector Location;
	FRotator Rotation;
	OwnerCharacter->GetController()->GetPlayerViewPoint(Location, Rotation);

	FVector DashDirection = Rotation.Vector();
	FVector DashVelocity = DashDirection * DashForce;
	float OriginalGroundFriction = OwnerCharacter->GetCharacterMovement()->GroundFriction;
	OwnerCharacter->GetCharacterMovement()->GroundFriction = 0.f;
	float DashDuration = FMath::Clamp(DashForce / 10000.f, 0.05f, 0.25f);
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer( Timer, FTimerDelegate::CreateLambda([this, OriginalGroundFriction]()
	{
		OwnerCharacter->GetCharacterMovement()->GroundFriction = OriginalGroundFriction;
	}), DashDuration, false);
	
	OwnerCharacter->LaunchCharacter(DashVelocity, true, true);
}

