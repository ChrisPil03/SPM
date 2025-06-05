#include "ObjectiveServer.h"

#include "NiagaraComponent.h"

AObjectiveServer::AObjectiveServer() :
	RestoreTime(40),
	RestoreProgress(FProgressTimer::ZeroCompletion),
	ServerState(EServerState::Idle),
	ProgressTimer(nullptr),
	FailTime(20)
	// bInstantRestoration(true),
	// HeatGeneration(3)
{
	PrimaryActorTick.bCanEverTick = true;
	SmokeNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("Smoke Niagara Component");
	SmokeNiagaraComponent->SetupAttachment(RootComponent);
	SmokeNiagaraComponent->bAutoActivate = false;
}

void AObjectiveServer::BeginPlay()
{
	Super::BeginPlay();

	// if (RestoreTime > 0.f)
	// {
	// 	bInstantRestoration = false;
	// }
	ProgressTimer = MakeUnique<FProgressTimer>(RestoreTime);
	FailProgressTimer = MakeUnique<FProgressTimer>(FailTime);
	FailProgressTimer->SetCompletionFunction(this, &AObjectiveServer::BroadcastServerFailed);
	SetCanInteractWith(false);
}

void AObjectiveServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsPaused())
	{
		IncreaseFailProgress(DeltaTime);
	}
	else if (GetIsRestoring())
	{
		IncreaseRestorationProgress(DeltaTime);
	}
}

void AObjectiveServer::Interact(AActor* Interactor)
{
	if (GetCanInteractWith())
	{
		// if (GetNeedsRestoring())
		// {
		// 	StartRestoration();
		// }
		SetCanInteractWith(false);
		ResumeRestoration();
		ResetMaterial();

		if (PerformDelegate.IsBound())
		{
			PerformDelegate.Broadcast(this);
		}
	}
}

void AObjectiveServer::SetServerState(const EServerState NewState)
{
	ServerState = NewState;

	if (GetNeedsRestoring())
	{
		SetPausedMaterials();
	}
}

void AObjectiveServer::StartRestoration()
{
	// UE_LOG(LogTemp, Warning, TEXT("Restoration Started"));

	// if (bInstantRestoration)
	// {
	// 	CompleteRestoration();
	// 	return;
	// }
	SetServerState(EServerState::Restoring);
	
	GetWorld()->GetTimerManager().SetTimer(
		MalfunctionTimerHandle,
		this,
		&AObjectiveServer::PauseRestoration,
		FMath::RandRange(1, RestoreTime - 1),
		false);
}

void AObjectiveServer::GetElapsedMinutesAndSeconds(int32& OutMinutes, int32& OutSeconds)
{
	if (FailProgressTimer)
	{
		FailProgressTimer->GetElapsedMinutesAndSeconds(OutMinutes, OutSeconds);
	}
}

void AObjectiveServer::IncreaseRestorationProgress(float DeltaTime)
{
	ProgressTimer->IncreaseProgress(DeltaTime);
	RestoreProgress = ProgressTimer->GetProgress();
	// GenerateHeat(DeltaTime);
		
	if (RestoreProgress == FProgressTimer::FullCompletion)
	{
		CompleteRestoration();
	}
}

void AObjectiveServer::IncreaseFailProgress(float DeltaTime)
{
	if (FailProgressTimer)
	{
		FailProgressTimer->IncreaseProgress(DeltaTime);
	}
}

void AObjectiveServer::BroadcastServerFailed()
{
	if (OnFailEvent.IsBound())
	{
		OnFailEvent.Broadcast(this);
	}
}

void AObjectiveServer::BroadcastServerPaused()
{
	if (OnPausedEvent.IsBound())
	{
		OnPausedEvent.Broadcast(this);
	}
}

void AObjectiveServer::BroadcastServerResumed()
{
	if (OnResumedEvent.IsBound())
	{
		OnResumedEvent.Broadcast(this);
	}
}

// void AObjectiveServer::GenerateHeat(float DeltaTime)
// {
// 	if (HeatUpDelegate.IsBound())
// 	{
// 		HeatUpDelegate.Execute(HeatGeneration * DeltaTime);
// 	}
// }

void AObjectiveServer::CompleteRestoration()
{
	// UE_LOG(LogTemp, Warning, TEXT("Restoration Complete"));
	SetServerState(EServerState::Restored);
	RestoreProgress = FProgressTimer::FullCompletion;
	
	if (CompleteDelegate.IsBound())
	{
		CompleteDelegate.Broadcast(this);
	}
	ResetMaterial();
	GetWorld()->GetTimerManager().ClearTimer(MalfunctionTimerHandle);
}

void AObjectiveServer::PauseRestoration()
{
	if (!GetIsPaused())
	{
		SetServerState(EServerState::Paused);
		ProgressTimer->SetIsPaused(true);
		SetCanInteractWith(true);
		SetPausedMaterials();
		BroadcastServerPaused();
		// UE_LOG(LogTemp, Warning, TEXT("Pause Restoration"));
	}
}

void AObjectiveServer::ResumeRestoration()
{
	if (!GetIsRestoring())
	{
		SetServerState(EServerState::Restoring);
		ProgressTimer->SetIsPaused(false);
		FailProgressTimer->ResetTimer();
		SetCanInteractWith(false);
		BroadcastServerResumed();
		// UE_LOG(LogTemp, Warning, TEXT("Resume Restoration"));
	}
}

void AObjectiveServer::ResetServer()
{
	Super::Reset();
	SetServerState(EServerState::Idle);
	ProgressTimer->ResetTimer();
	FailProgressTimer->ResetTimer();
	RestoreProgress = 0;
	ResetMaterial();
	SetCanInteractWith(false);
	SetSmokeEffectActive(false);
	GetWorld()->GetTimerManager().ClearTimer(MalfunctionTimerHandle);
}

void AObjectiveServer::SetSmokeEffectActive(const bool bNewState) const
{
	if (SmokeNiagaraComponent)
	{
		if (bNewState)
		{
			SmokeNiagaraComponent->DeactivateImmediate();
			SmokeNiagaraComponent->Activate(bNewState);
		}else
		{
			SmokeNiagaraComponent->Deactivate();
		}
	}
}

void AObjectiveServer::SetPausedMaterials()
{
	if (CablePausedMaterial)
	{
		GetMesh()->SetMaterial(1, CablePausedMaterial);
		SetServerLightColor(false);
	}else
	{
		// UE_LOG(LogTemp, Warning, TEXT("Red meterial not set"));
	}
}

void AObjectiveServer::ResetMaterial()
{
	if (StandardMaterial)
	{
		GetMesh()->SetMaterial(1, StandardMaterial);
		SetServerLightColor(true);
	} else
	{
		// UE_LOG(LogTemp, Warning, TEXT("Standard meterial not set"));
	}
}