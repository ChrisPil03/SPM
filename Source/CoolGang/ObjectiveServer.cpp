#include "ObjectiveServer.h"

AObjectiveServer::AObjectiveServer() :
	RestoreTime(0.f),
	RestoreProgress(FProgressTimer::ZeroCompletion),
	ServerState(EServerState::Idle),
	ProgressTimer(nullptr),
	bInstantRestoration(true),
	HeatGeneration(3)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AObjectiveServer::BeginPlay()
{
	Super::BeginPlay();

	if (RestoreTime > 0.f)
	{
		bInstantRestoration = false;
	}
	ProgressTimer = MakeUnique<FProgressTimer>(RestoreTime);
}

void AObjectiveServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsPaused())
	{
		return;
	}

	if (GetIsRestoring())
	{
		IncreaseRestorationProgress(DeltaTime);
	}
}

void AObjectiveServer::Interact(AActor* Interactor)
{
	if (GetCanInteractWith())
	{
		if (GetNeedsRestoring())
		{
			StartRestoration();
		}
		SetCanInteractWith(false);
	}
}

void AObjectiveServer::SetServerState(const EServerState NewState)
{
	ServerState = NewState;

	if (GetNeedsRestoring())
	{
		SetDebugMaterial();
	}
}

void AObjectiveServer::StartRestoration()
{
	UE_LOG(LogTemp, Warning, TEXT("Restoration Started"));

	if (bInstantRestoration)
	{
		CompleteRestoration();
		return;
	}
	SetServerState(EServerState::Restoring);
}

void AObjectiveServer::IncreaseRestorationProgress(float DeltaTime)
{
	ProgressTimer->IncreaseProgress(DeltaTime);
	RestoreProgress = ProgressTimer->GetProgress();
	GenerateHeat(DeltaTime);
		
	if (RestoreProgress == FProgressTimer::FullCompletion)
	{
		CompleteRestoration();
	}
}

void AObjectiveServer::GenerateHeat(float DeltaTime)
{
	if (HeatUpDelegate.IsBound())
	{
		HeatUpDelegate.Execute(HeatGeneration * DeltaTime);
	}
}

void AObjectiveServer::CompleteRestoration()
{
	UE_LOG(LogTemp, Warning, TEXT("Restoration Complete"));
	SetServerState(EServerState::Restored);
	RestoreProgress = FProgressTimer::FullCompletion;

	if (PerformDelegate.IsBound())
	{
		PerformDelegate.Broadcast(this);
	}
	ResetMaterial();
}

void AObjectiveServer::PauseRestoration()
{
	if (!GetIsPaused())
	{
		SetServerState(EServerState::Paused);
		ProgressTimer->SetIsPaused(true);
		UE_LOG(LogTemp, Warning, TEXT("Pause Restoration"));
	}
}

void AObjectiveServer::ResumeRestoration()
{
	if (!GetIsRestoring())
	{
		SetServerState(EServerState::Restoring);
		ProgressTimer->SetIsPaused(false);
		UE_LOG(LogTemp, Warning, TEXT("Resume Restoration"));
	}
}

void AObjectiveServer::ResetServer()
{
	Super::Reset();
	SetServerState(EServerState::Idle);
	ProgressTimer->Reset();
	RestoreProgress = 0;
	ResetMaterial();
}

void AObjectiveServer::SetDebugMaterial() const 
{
	if (RedMaterial)
	{
		GetMesh()->SetMaterial(0, RedMaterial);
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("Red meterial not set"));
	}
}

void AObjectiveServer::ResetMaterial()
{
	if (StandardMaterial)
	{
		GetMesh()->SetMaterial(0, StandardMaterial);
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("Standard meterial not set"));
	}
}