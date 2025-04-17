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
	StartLocation = OwnerCharacter->GetActorLocation();
	
	Rotation.Pitch = 0.f; // Ignore vertical aim
	FVector DashDirection = Rotation.Vector();
	FVector DashVelocity = DashDirection * DashForce;
	DashVelocity.Z = 0.f;
	OwnerCharacter->GetCharacterMovement()->GroundFriction = 0;
	bIsDashing = true;
	OwnerCharacter->LaunchCharacter(DashVelocity, true, true);
	
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
	
}

void UDashComponent::CheckToReset()
{
	if (!OwnerCharacter || !bIsDashing)
		return;

	FVector CurrentLocation = OwnerCharacter->GetActorLocation();
	float TraveledDistance = FVector::Dist(StartLocation, CurrentLocation);

	if (TraveledDistance >= DashDistance || !GetWorld()->GetTimerManager().IsTimerActive(DashTimer))
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
	//OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
	
	UE_LOG(LogTemp, Warning, TEXT("Dash Distance: a"));
	
}

