// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveCapture.h"
#include "PlayerCharacter.h"
#include "Components/SphereComponent.h"

AObjectiveCapture::AObjectiveCapture()
{
	CaptureRadius = 600.f;
	PlayerTag = "Player";
	PlayerInZone = nullptr;
	SetIsTimeBased(true);
	
	SphereTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Trigger Component"));
	SphereTrigger->InitSphereRadius(CaptureRadius);
	RootComponent = SphereTrigger;
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
	BaseMesh->SetupAttachment(SphereTrigger);
}

void AObjectiveCapture::BeginPlay()
{
	Super::BeginPlay();
	SphereTrigger->OnComponentBeginOverlap.AddDynamic(this, &AObjectiveCapture::OnSphereBeginOverlap);
	SphereTrigger->OnComponentEndOverlap.AddDynamic(this, &AObjectiveCapture::OnSphereEndOverlap);
}

void AObjectiveCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GetIsAborting())
	{
		DecreaseObjectiveProgress(DeltaTime);
	}
}

void AObjectiveCapture::Interact(AActor* Interactor)
{
	if (!GetIsActive())
	{
		return;
	}
	if (GetIsNotStarted())
	{
		StartObjective();
	}
}

void AObjectiveCapture::StartObjective()
{
	Super::StartObjective();
	SpawnCaptureZone();
}

void AObjectiveCapture::ResetObjective()
{
	Super::ResetObjective();
	DestroyCaptureZone();
}

void AObjectiveCapture::IncreaseObjectiveProgress(float const DeltaTime)
{
	Super::IncreaseObjectiveProgress(DeltaTime);
	UpdateCaptureZoneSize();

	if (GetObjectiveProgress() == FProgressTimer::FullCompletion)
	{
		CompleteObjective();
	}
}

void AObjectiveCapture::DecreaseObjectiveProgress(float const DeltaTime)
{
	Super::DecreaseObjectiveProgress(DeltaTime);
	UpdateCaptureZoneSize();

	if (GetObjectiveProgress() == FProgressTimer::ZeroCompletion)
	{
		ResetObjective();
	}
}

void AObjectiveCapture::OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		PlayerInZone = Player;
		
		if (GetIsAborting())
		{
			SetObjectiveState(EObjectiveState::InProgress);
		}
	}
}

void AObjectiveCapture::OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	
	if (OtherActor == PlayerInZone)
	{
		PlayerInZone = nullptr;

		if (GetIsInProgress())
		{
			SetObjectiveState(EObjectiveState::Aborting);
		}
	}
}
