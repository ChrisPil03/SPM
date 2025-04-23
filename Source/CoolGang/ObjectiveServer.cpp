#include "ObjectiveServer.h"

AObjectiveServer::AObjectiveServer() :
	RestoreTime(0.f),
	RestoreProgress(FProgressTimer::ZeroCompletion),
	ServerState(EServerState::Idle),
	ProgressTimer(nullptr),
	bInstantRestoration(true),
	HeatGeneration(3),
	CoolingTime(3)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AObjectiveServer::BeginPlay()
{
	Super::BeginPlay();

	if (RestoreTime > 0.f)
	{
		bInstantRestoration = false;
		InitiateTimers();
	}
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

	if (GetIsCooling())
	{
		CoolDown(DeltaTime);
	}
}

void AObjectiveServer::InitiateTimers()
{
	ProgressTimer = MakeUnique<FProgressTimer>(RestoreTime);
	CoolingTimer = MakeUnique<FProgressTimer>(CoolingTime);

	FTimerCompletionDelegate CoolingDelegate;
	CoolingDelegate.BindUObject(this, &AObjectiveServer::ResumeRestoration);
	CoolingTimer->SetCompletionDelegate(CoolingDelegate);
}

void AObjectiveServer::Interact(AActor* Interactor)
{
	if (GetNeedsRestoring())
	{
		StartRestoration();
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

	UE_LOG(LogTemp, Warning, TEXT("Progress: %f"), RestoreProgress);
		
	if (RestoreProgress == FProgressTimer::FullCompletion)
	{
		CompleteRestoration();
	}
}

void AObjectiveServer::BeginCooling()
{
	SetServerState(EServerState::Cooling);
}

void AObjectiveServer::CoolDown(float DeltaTime)
{
	CoolingTimer->IncreaseProgress(DeltaTime);
	UE_LOG(LogTemp, Warning, TEXT("Cooling progress: %f"), CoolingTimer->GetProgress());
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
}

void AObjectiveServer::PauseRestoration()
{
	ProgressTimer->SetIsPaused(true);
	SetServerState(EServerState::Paused);
}

void AObjectiveServer::ResumeRestoration()
{
	if (GetIsPaused() || GetIsCooling())
	{
		ProgressTimer->SetIsPaused(false);
		SetServerState(EServerState::Restoring);	
	}
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
