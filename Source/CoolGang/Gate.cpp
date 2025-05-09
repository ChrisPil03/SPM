#include "Gate.h"

#include "MoverComponent.h"
#include "Components/BoxComponent.h"

AGate::AGate()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gate Mesh Component"));
	GateMesh->SetupAttachment(RootComponent);

	LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Door Component"));
	LeftDoorMesh->SetupAttachment(RootComponent);

	RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Door Component"));
	RightDoorMesh->SetupAttachment(RootComponent);

	LeftMover = CreateDefaultSubobject<UMoverComponent>("Left Mover Component");
	RightMover = CreateDefaultSubobject<UMoverComponent>("Right Mover Component");

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("Box Component");
	BoxComponent->SetupAttachment(RootComponent);
}

void AGate::BeginPlay()
{
	Super::BeginPlay();
	if (LeftMover)
	{
		LeftMover->SetMeshComponentToMove(LeftDoorMesh);	
	}
	if (RightMover)
	{
		RightMover->SetMeshComponentToMove(RightDoorMesh);	
	}
}

void AGate::OpenGate() const
{
	LeftMover->SetShouldMove(true);
	RightMover->SetShouldMove(true);
	LeftMover->ReverseDirection(false);
	RightMover->ReverseDirection(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGate::CloseGate() const
{
	LeftMover->SetShouldMove(true);
	RightMover->SetShouldMove(true);
	LeftMover->ReverseDirection(true);
	RightMover->ReverseDirection(true);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}