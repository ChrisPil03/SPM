#include "ObjectiveCapture.h"
#include "InteractableObject.h"
#include "PlayerCharacter.h"
#include "ScoreManagerComponent.h"
#include "Components/SphereComponent.h"

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
}

void AObjectiveCapture::BeginPlay()
{
	Super::BeginPlay();
	FailDelayProgressTimer = MakeUnique<FProgressTimer>(FailObjectiveDelay);
	FailDelayProgressTimer->SetCompletionFunction(this, &AObjectiveCapture::FailObjective);
	SphereTrigger->OnComponentBeginOverlap.AddDynamic(this, &AObjectiveCapture::OnSphereBeginOverlap);
	SphereTrigger->OnComponentEndOverlap.AddDynamic(this, &AObjectiveCapture::OnSphereEndOverlap);

	FindInteractable();
	BindInteractable();
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

		if (OnUniqueProgressChanged.IsBound())
		{
			OnUniqueProgressChanged.Broadcast();
		}
	}
}

void AObjectiveCapture::SetIsActive(const bool bNewState)
{
	Super::SetIsActive(bNewState);
	if (ControlPanel)
	{
		ControlPanel->SetCanInteractWith(bNewState);	
	}
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
	OnRequestAddScore.Broadcast(EScoreType::ObjectiveDownloadCompleted);
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

FVector AObjectiveCapture::GetWaypointTargetLocation() const
{
	if (ControlPanel)
	{
		// FVector Origin;
		// FVector Extent;
		// ControlPanel->GetActorBounds(false, Origin, Extent, false);
		return ControlPanel->GetActorLocation();
	}
	return Super::GetWaypointTargetLocation();
}

TArray<FString> AObjectiveCapture::GetUniqueObjectiveProgress() const
{
	if (!GetIsPlayerInZone() && GetIsInProgress())
	{
		int32 OutMinutes;
		int32 OutSeconds;
		GetTimeUntilFailure(OutMinutes, OutSeconds);
		FString Minutes;
		FString Seconds;
		if (OutMinutes <= 9)
		{
			Minutes = FString::Printf(TEXT("0%d"), OutMinutes);
		}else
		{
			Minutes = FString::Printf(TEXT("%d"), OutMinutes);
		}
		if (OutSeconds <= 9)
		{
			Seconds = FString::Printf(TEXT("0%d"), OutSeconds);
		}else
		{
			Seconds = FString::Printf(TEXT("%d"), OutSeconds);
		}
		return {
			FString::Printf(TEXT("Download failure in: %s:%s"), *Minutes, *Seconds)
		};
	}
	return Super::GetUniqueObjectiveProgress();
}

void AObjectiveCapture::GetTimeUntilFailure(int32& OutMinutes, int32& OutSeconds) const
{
	int32 ElapsedTime = GetObjectiveTime() - GetProgressTimer().GetElapsedTime();
	int32 FailElapsedTime = FailDelayProgressTimer->GetElapsedTime();
	int32 TimeLeft = GetObjectiveTime() + FailObjectiveDelay - ElapsedTime - FailElapsedTime;
	OutMinutes = TimeLeft / 60;
	OutSeconds = TimeLeft % 60;
}

void AObjectiveCapture::FindInteractable()
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (AActor* Actor : AttachedActors)
	{
		if (AInteractableObject* Interactable = Cast<AInteractableObject>(Actor))
		{
			ControlPanel = Interactable;
			break;
		}
	}
}

void AObjectiveCapture::BindInteractable()
{
	if (ControlPanel)
	{
		ControlPanel->SetOnInteractFunction(this, &AObjectiveCapture::RegisterControlPanelInteraction);	
	}
}

void AObjectiveCapture::RegisterControlPanelInteraction(AInteractableObject* Interactable)
{
	StartObjective();
	// ControlPanel->SetCanInteractWith(false);
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

		if (OnUniqueProgressChanged.IsBound())
		{
			OnUniqueProgressChanged.Broadcast();
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
			EnqueueVoiceLineWithMessage(ExitZoneVoiceLine, "STAY IN THE ZONE");
		}
	}
}
