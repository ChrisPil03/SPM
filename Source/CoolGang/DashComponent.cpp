// Fill out your copyright notice in the Description page of Project Settings.


#include "DashComponent.h"

#include "PlayerCharacter.h"
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
	CharacterMovement = OwnerCharacter->GetCharacterMovement();
	OriginalGroundFriction = CharacterMovement->GroundFriction;
	
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

	GetWorld()->GetTimerManager().SetTimer(CooldownTimer, FTimerDelegate::CreateLambda([this](){}), Cooldown, false);
	GetWorld()->GetTimerManager().SetTimer(DashTimer, FTimerDelegate::CreateLambda([this](){}), StopTime, false);
	
	FVector Location;
	FRotator Rotation;
	OwnerCharacter->GetController()->GetPlayerViewPoint(Location, Rotation);
	StartLocation = OwnerCharacter->GetActorLocation();
	
	Rotation.Pitch = 0.f; // Ignore vertical aim
	DashDirection = OwnerCharacter->GetLastMovementInputVector();
	FVector DashVelocity = DashDirection;
	
	if (DashDirection == FVector::ZeroVector)
	{
		DashVelocity = Rotation.Vector();
	}
	DashVelocity *= DashForce;
	CharacterMovement->GroundFriction = 0;
	bIsDashing = true;
	CharacterMovement->StopMovementImmediately();
	CharacterMovement->Velocity = DashVelocity;
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerCharacter))
	{
		Player->OnDash();
	}

	//OwnerCharacter->LaunchCharacter(DashVelocity, false, false);
	
	
}

void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if (bIsDashing)
	{
		if (CheckToReset())
		{
			Reset();
			// UE_LOG(LogTemp, Warning, TEXT("Reset dash"));
		}
	}
	if (!bIsDashing && bShouldDecelerate)
	{
		CurrentVelocity = OwnerCharacter->GetVelocity();
		CurrentVelocity.Z = 0.0f;
		TargetVelocity = OwnerCharacter->GetActorForwardVector() * CharacterMovement->MaxWalkSpeed;
		NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, DecelerationSpeed);
		
		NewVelocity.Z = OwnerCharacter->GetVelocity().Z;
		CharacterMovement->Velocity = NewVelocity;
		
		if (CurrentVelocity.SizeSquared() <= (CharacterMovement->MaxWalkSpeed * CharacterMovement->MaxWalkSpeed))
		{
			bShouldDecelerate = false; // Stop decelerating
			CharacterMovement->GroundFriction = OriginalGroundFriction;
			// UE_LOG(LogTemp, Warning, TEXT(" Stop decelerating"));
		}
	}
}

bool UDashComponent::CheckToReset()
{
	if (!OwnerCharacter || !bIsDashing)
		return false;

	if (GetWorld()->GetTimerManager().IsTimerActive(DashTimer))
	{
		return true;
	}
	return false;
}

void UDashComponent::Reset()
{
	bIsDashing = false;
	// Restore friction now
	
	
	// Stop movement by resetting velocity
	bShouldDecelerate = true;
	
	
	
}

