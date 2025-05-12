#include "MoverComponent.h"

UMoverComponent::UMoverComponent() :
	ShouldMove(false),
	MoveSpeed(3.f),
	MoveOffset(FVector::ZeroVector),
	MeshComponentToMove(nullptr),
	OriginalPosition(FVector::ZeroVector),
	TargetLocation(FVector::ZeroVector)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMoverComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShouldMove && MeshComponentToMove)
	{
		MoveObject(DeltaTime);	
	}
}

void UMoverComponent::SetMeshComponentToMove(UStaticMeshComponent* MeshComponent)
{
	MeshComponentToMove = MeshComponent;

	if (MeshComponentToMove)
	{
		OriginalPosition = MeshComponentToMove->GetRelativeLocation();
		// UE_LOG(LogTemp, Warning, TEXT("RelativeLocation: %f, %f, %f"), OriginalPosition.X, OriginalPosition.Y, OriginalPosition.Z);
		TargetLocation = OriginalPosition + MoveOffset;
	}
}

void UMoverComponent::ReverseDirection(const bool bBackToOriginal)
{
	if (!MeshComponentToMove)
	{
		return;
	}
	if (bBackToOriginal)
	{
		TargetLocation = OriginalPosition;
	}else
	{
		TargetLocation = OriginalPosition + MoveOffset;
	}
}

void UMoverComponent::MoveObject(const float DeltaTime)
{
	const FVector CurrentLocation = MeshComponentToMove->GetRelativeLocation();
	const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
	MeshComponentToMove->SetRelativeLocation(NewLocation);

	if (MeshComponentToMove->GetRelativeLocation() == TargetLocation)
	{
		ShouldMove = false;
	}
}

