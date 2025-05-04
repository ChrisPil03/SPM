
#include "ObjectiveRestoreServers.h"
#include "ObjectiveServer.h"
#include "PlayerLocationDetection.h"
#include "Kismet/GameplayStatics.h"

AObjectiveRestoreServers::AObjectiveRestoreServers() :
	RestoredServers(0),
	NumberOfServers(0),
	NumberOfServersToRestore(3),
	bCanOverheat(true),
	ServerHallStatus(EServerHallStatus::Operating),
	MaxHeatBuildup(100.f),
	CurrentHeatBuildup(0.f),
	CoolingTime(3.f),
	CoolingProgress(0.f),
	OverheatSystemIntegrityDamage(50.f),
	FailDelay(15.f),
	OverheatedMessage("SERVERS OVERHEATING")
{
	ServersToRestore.Reserve(NumberOfServersToRestore);
	SetIsTimeBased(false);
}

void AObjectiveRestoreServers::BeginPlay()
{
	Super::BeginPlay();
	InitializeServerHall();
	ControlPanel->SetCanInteractWith(false);
}

void AObjectiveRestoreServers::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsComplete() || GetIsFailed())
	{
		return;
	}
	if (GetIsOperating())
	{
		IncreaseObjectiveProgress(DeltaTime);
	}
	else if (GetIsOverheated())
	{
		if (!FailDelayProgressTimer)
		{
			return;
		}
		FailDelayProgressTimer->IncreaseProgress(DeltaTime);
	}
	else if (GetIsCooling())
	{
		CoolDown(DeltaTime);
	}
}

void AObjectiveRestoreServers::SetIsActive(const bool bNewState)
{
	UE_LOG(LogTemp, Warning, TEXT("SetIsActive Restore servers"));
	Super::SetIsActive(bNewState);
	
	if (bNewState)
	{
		SelectServersToRestore();
		PrepareServersToRestore();
	}
}

void AObjectiveRestoreServers::FailObjective()
{
	Super::FailObjective();
	ResetServerRoom();
}

void AObjectiveRestoreServers::InitializeServerHall()
{
	FindAllServers();
	BindControlPanel();
	InitializeTimers();
}

void AObjectiveRestoreServers::SelectServersToRestore()
{
	if (NumberOfServers < NumberOfServersToRestore)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected Servers to Restore: %d, %d"), NumberOfServersToRestore, NumberOfServers);
		return;
	}

	for (int32 i = 0; i < NumberOfServersToRestore; ++i)
	{
		int32 RandomIndex = FMath::RandRange(i, NumberOfServers - 1);
		AllServers.Swap(i, RandomIndex);
	}
	ServersToRestore.Append(&AllServers[0], NumberOfServersToRestore);
}

void AObjectiveRestoreServers::PrepareServersToRestore()
{
	if (ServersToRestore.Num() <= 0)
	{
		return;
	}
	
	// FPerformDelegate RestoredDelegate;
	// RestoredDelegate.AddUObject(this, &AObjectiveRestoreServers::RegisterServerRestored);
	FPerformDelegate InteractDelegate;
	InteractDelegate.AddUObject(this, &AObjectiveRestoreServers::OnInteract);
	FServerHeatUpDelegate HeatUpDelegate;
	HeatUpDelegate.BindUObject(this, &AObjectiveRestoreServers::AddHeatBuildup);
	
	for (AObjectiveServer* Server : ServersToRestore)
	{
		Server->SetCanInteractWith(true);
		Server->SetServerState(EServerState::NeedRestoring);
		Server->SetInteractFunction(InteractDelegate);
		Server->SetHeatUpFunction(HeatUpDelegate);
		Server->SetCompleteObjectiveFunction(this, &AObjectiveRestoreServers::RegisterServerRestored);
	}
	UE_LOG(LogTemp, Warning, TEXT("Prepared Servers to Restore"));
}

void AObjectiveRestoreServers::FindAllServers()
{
	TArray<AActor*> ServerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AObjectiveServer::StaticClass(), ServerActors);
	
	AllServers.Reserve(ServerActors.Num());
	
	for (AActor* Actor : ServerActors)
	{
		if (AObjectiveServer* Server = Cast<AObjectiveServer>(Actor))
		{
			Server->SetCanInteractWith(false);
			AllServers.Add(Server);
		}
	}
	NumberOfServers = AllServers.Num();
}

void AObjectiveRestoreServers::BindControlPanel()
{
	if (ControlPanel)
	{
		ControlPanel->SetOnInteractFunction(this, &AObjectiveRestoreServers::RegisterControlPanelInteraction);
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectiveRestoreServers: ControlPanel is missing"));
	}
}

void AObjectiveRestoreServers::ResetServersToRestore()
{
	for (AObjectiveServer* Server : ServersToRestore)
	{
		Server->ResetServer();
	}
	ServersToRestore.Empty();
	ServersToRestore.Reserve(NumberOfServersToRestore);
	RestoredServers = 0;
}

void AObjectiveRestoreServers::ResetServerRoom()
{
	SetServerHallStatus(EServerHallStatus::Operating);
	ResetHeatBuildup();
	ActivateControlPanel(false);
	CoolingProgress = 0;
	CoolingTimer->ResetTimer();
	FailDelayProgressTimer->ResetTimer();
	ResetServersToRestore();
}

void AObjectiveRestoreServers::OnInteract(AInteractableObject* InteractableObject)
{
	StartObjective();
}

void AObjectiveRestoreServers::ActivateControlPanel(const bool NewState)
{
	if (ControlPanel)
	{
		ControlPanel->SetCanInteractWith(NewState);
	}
}

bool AObjectiveRestoreServers::ValidServerToRestore(const AObjectiveServer* Server) const
{
	return Server && !Server->GetIsRestored();
}

void AObjectiveRestoreServers::RegisterServerRestored(AInteractableObject* InteractableObject)
{
	if (AObjectiveServer* Server = Cast<AObjectiveServer>(InteractableObject))
	{
		if (ServersToRestore.Contains(Server))
		{
			++RestoredServers;
		}
	}
	if (GetIsServersRestored())
	{
		CompleteObjective();
	}
}

void AObjectiveRestoreServers::RegisterServerInteraction(AInteractableObject* InteractableObject)
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

void AObjectiveRestoreServers::RegisterControlPanelInteraction(AInteractableObject* InteractableObject)
{
	if (!GetIsActive() && !GetIsOverheated())
	{
		return;
	}
	InitiateCoolingCycle();
}

void AObjectiveRestoreServers::ResetObjective()
{
	Super::ResetObjective();
	ResetServerRoom();
}

void AObjectiveRestoreServers::CompleteObjective()
{
	Super::CompleteObjective();
	ResetHeatBuildup();
	SetObjectiveProgress(1.f);

	if (ControlPanel)
	{
		ControlPanel->SetCanInteractWith(false);
	}
}

void AObjectiveRestoreServers::IncreaseObjectiveProgress(float const DeltaTime)
{
	float ObjectiveProgress = 0;
	for (AObjectiveServer* Server : ServersToRestore)
	{
		ObjectiveProgress += Server->GetProgress();
	}
	ObjectiveProgress /= NumberOfServersToRestore;
	SetObjectiveProgress(ObjectiveProgress);
}

// ________________________________________________________________
// ____________________ Overheat / Cooling ________________________

void AObjectiveRestoreServers::AddHeatBuildup(float Heat)
{
	if (!bCanOverheat || GetIsOverheated() || GetIsCooling())
	{
		return;
	}
	CurrentHeatBuildup += Heat;

	if (CurrentHeatBuildup >= MaxHeatBuildup)
	{
		CurrentHeatBuildup = MaxHeatBuildup;
		TriggerOverheat();
	}
}

void AObjectiveRestoreServers::TriggerOverheat()
{
	if (GetIsOverheated())
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Trigger Overheat"));
	SetServerHallStatus(EServerHallStatus::Overheated);
	ActivateControlPanel(true);
	DisplayMessage(OverheatedMessage);

	for (AObjectiveServer* Server : ServersToRestore)
	{
		if (ValidServerToRestore(Server))
		{
			Server->PauseRestoration();
		}
	}
}

void AObjectiveRestoreServers::InitiateCoolingCycle()
{
	if (!GetIsCooling() && GetIsInProgress())
	{
		if (FailDelayProgressTimer->GetProgress() > FProgressTimer::ZeroCompletion)
		{
			FailDelayProgressTimer->ResetTimer();
		}
		SetServerHallStatus(EServerHallStatus::Cooling);
	}
}

void AObjectiveRestoreServers::CoolDown(float DeltaTime)
{
	CoolingTimer->IncreaseProgress(DeltaTime);
	CoolingProgress = CoolingTimer->GetProgress();
	CurrentHeatBuildup -= CoolingProgress;
	CurrentHeatBuildup = FMath::Clamp(CurrentHeatBuildup, 0.f, MaxHeatBuildup);
}

void AObjectiveRestoreServers::ResumeOperating()
{
	UE_LOG(LogTemp, Warning, TEXT("Resume Operating"));
	if (GetIsCooling())
	{
		SetServerHallStatus(EServerHallStatus::Operating);
		ResetHeatBuildup();
		CoolingTimer->ResetTimer();

		for (AObjectiveServer* Server : ServersToRestore)
		{
			if (ValidServerToRestore(Server) && Server->GetIsPaused())
			{
				Server->ResumeRestoration();
			}
		}
	}
}

void AObjectiveRestoreServers::InitializeTimers()
{
	CoolingTimer = MakeUnique<FProgressTimer>(CoolingTime);
	CoolingTimer->SetCompletionFunction(this, &AObjectiveRestoreServers::ResumeOperating);
	FailDelayProgressTimer = MakeUnique<FProgressTimer>(FailDelay);
	FailDelayProgressTimer->SetCompletionFunction(this, &AObjectiveRestoreServers::FailObjective);
	if (FailDelayProgressTimer)
	{
		UE_LOG(LogTemp, Error, TEXT("FailDelayProgressTimer != nullptr"));
	}
}

void AObjectiveRestoreServers::ResetHeatBuildup()
{
	CurrentHeatBuildup = 0.f;
}