#include "ObjectiveServer.h"

AObjectiveServer::AObjectiveServer() :
	RestoreTime(0.f),
	RestoreProgress(FProgressTimer::ZeroCompletion),
	ServerState(EServerState::Idle),
	Timer(nullptr),
	bInstantRestoration(true)
{
	PrimaryActorTick.bCanEverTick = true;
}

AObjectiveServer::~AObjectiveServer()
{
	if (Timer)
	{
		delete Timer;
		Timer = nullptr;
	}
}

void AObjectiveServer::BeginPlay()
{
	Super::BeginPlay();

	if (RestoreTime > 0.f)
	{
		bInstantRestoration = false;
		Timer = new FProgressTimer(RestoreTime);
		if (!Timer)
		{
			UE_LOG(LogTemp, Warning, TEXT("ObjectiveServer: Timer is nullptr"));
		}
	}
}

void AObjectiveServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsRestoring() && Timer)
	{
		IncreaseRestorationProgress(DeltaTime);
	}
}

void AObjectiveServer::Interact(AActor* Interactor)
{
	//Super::Interact(Interactor);

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
	if (Timer)
	{
		Timer->IncreaseProgress(DeltaTime);
		RestoreProgress = Timer->GetProgress();

		UE_LOG(LogTemp, Warning, TEXT("Progress: %f"), RestoreProgress);
		
		if (RestoreProgress == FProgressTimer::FullCompletion)
		{
			CompleteRestoration();
		}
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