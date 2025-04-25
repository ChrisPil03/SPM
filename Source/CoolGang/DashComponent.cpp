// Fill out your copyright notice in the Description page of Project Settings.


#include "DashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UDashComponent::UDashComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	OriginalGroundFriction = OwnerCharacter->GetCharacterMovement()->GroundFriction;
}




void UDashComponent::Dash()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(CooldownTimer))
	{
		return;
	}
	if (!OwnerCharacter)
	{
		return;
	}
	
	FVector Location;
	FRotator Rotation;
	OwnerCharacter->GetController()->GetPlayerViewPoint(Location, Rotation);
	
	
	Rotation.Pitch = 0.f; // Ignore vertical aim
	FVector DashDirection = OwnerCharacter->GetLastMovementInputVector();
	FVector DashVelocity = DashDirection;

	if (DashDirection == FVector::ZeroVector)
	{
		DashVelocity = Rotation.Vector();
	}
	DashVelocity *= DashForce;
	OwnerCharacter->GetCharacterMovement()->GroundFriction = 0;
	bIsDashing = true;
	OwnerCharacter->LaunchCharacter(DashVelocity, false, false);
	
	GetWorld()->GetTimerManager().SetTimer(CooldownTimer, FTimerDelegate::CreateLambda([this](){}), Cooldown, false);
	GetWorld()->GetTimerManager().SetTimer(DashTimer, FTimerDelegate::CreateLambda([this](){}), StopTime, false);
}

void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if (bIsDashing)
	{
		CheckToReset();
	}
	if (!bIsDashing && bShouldDecelerate)
	{
		FVector CurrentVelocity = OwnerCharacter->GetVelocity();
		FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, FVector::ZeroVector, DeltaTime, DecelerationSpeed);
		OwnerCharacter->GetCharacterMovement()->Velocity = NewVelocity;

		if (NewVelocity.SizeSquared() < 10.f)
		{
			bShouldDecelerate = false; // Stop decelerating
		}
	}
}

void UDashComponent::CheckToReset()
{
	if (!OwnerCharacter || !bIsDashing)
		return;

	

	if (!GetWorld()->GetTimerManager().IsTimerActive(DashTimer))
	{
		Reset();
	}
}

void UDashComponent::Reset()
{
	bIsDashing = false;
	// Restore friction now
	OwnerCharacter->GetCharacterMovement()->GroundFriction = OriginalGroundFriction;
	
	// Stop movement by resetting velocity
	bShouldDecelerate = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Reset dash"));
	
}

