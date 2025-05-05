// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveCapture.h"
#include "PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AObjectiveCapture::AObjectiveCapture()
{
	CaptureRadius = 600.f;
	PlayerTag = "Player";
	PlayerInZone = nullptr;
	SetIsTimeBased(true);
	bCanInteractWith = false;
	DestroyZoneDelay = 1;
	FailObjectiveDelay = 5.f;
	
	SphereTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Trigger Component"));
	SphereTrigger->InitSphereRadius(CaptureRadius);
	RootComponent = SphereTrigger;
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
	BaseMesh->SetupAttachment(SphereTrigger);
}

void AObjectiveCapture::BeginPlay()
{
	Super::BeginPlay();
	FailDelayProgressTimer = MakeUnique<FProgressTimer>(FailObjectiveDelay);
	FailDelayProgressTimer->SetCompletionFunction(this, &AObjectiveCapture::FailObjective);
	SphereTrigger->OnComponentBeginOverlap.AddDynamic(this, &AObjectiveCapture::OnSphereBeginOverlap);
	SphereTrigger->OnComponentEndOverlap.AddDynamic(this, &AObjectiveCapture::OnSphereEndOverlap);
}

void AObjectiveCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetIsInProgress())
	{
		return;
	}
	if (GetIsPlayerInZone())
	{
		if (FailDelayProgressTimer->GetProgress() > 0)
		{
			FailDelayProgressTimer->ResetTimer();
		}
		IncreaseObjectiveProgress(DeltaTime);
	}
	else
	{
		if (GetObjectiveProgress() == FProgressTimer::ZeroCompletion)
		{
			FailDelayProgressTimer->IncreaseProgress(DeltaTime);
		}else
		{
			DecreaseObjectiveProgress(DeltaTime);
		}
	}
}

void AObjectiveCapture::Interact(AActor* Interactor)
{
	if (!bCanInteractWith || !GetIsActive())
	{
		return;
	}
	if (GetIsNotStarted())
	{
		StartObjective();
	}
	SetCanInteractWith(false);
}

void AObjectiveCapture::SetIsActive(const bool bNewState)
{
	Super::SetIsActive(bNewState);
	SetCanInteractWith(bNewState);
}

void AObjectiveCapture::StartObjective()
{
	Super::StartObjective();
	SpawnCaptureZone();
}

void AObjectiveCapture::CompleteObjective()
{
	Super::CompleteObjective();
	GetWorld()->GetTimerManager().SetTimer(
		DelayTimerHandle, this, &AObjectiveCapture::DestroyCaptureZone, DestroyZoneDelay, false);
}

void AObjectiveCapture::ResetObjective()
{
	Super::ResetObjective();
	//DestroyCaptureZone();
	//SetCanInteractWith(true);
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
	if (GetObjectiveProgress() > FProgressTimer::ZeroCompletion)
	{
		Super::DecreaseObjectiveProgress(DeltaTime);
		UpdateCaptureZoneSize();
	}
}

void AObjectiveCapture::FailObjective()
{
	if (!GetIsPlayerInZone())
	{
		Super::FailObjective();
		DestroyCaptureZone();
	}
}

void AObjectiveCapture::ShowInteractableOutline(const bool bNewState)
{
	BaseMesh->bRenderCustomDepth = bNewState;
	BaseMesh->MarkRenderStateDirty();
}

void AObjectiveCapture::SetCanInteractWith(const bool bNewState)
{
	bCanInteractWith = bNewState;
	ShowInteractableOutline(bNewState);
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
	}
}
