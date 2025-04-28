
#include "ObjectiveRestoreServers.h"
#include "ObjectiveServer.h"
#include "PlayerLocationDetection.h"
#include "Kismet/GameplayStatics.h"

AObjectiveRestoreServers::AObjectiveRestoreServers() :
	PlayerLocationDetection(nullptr),
	RestoredServers(0),
	NumberOfServers(0),
	NumberOfServersToRestore(3),
	bCanOverheat(true),
	ServerHallStatus(EServerHallStatus::Operating),
	MaxHeatBuildup(100.f),
	CurrentHeatBuildup(0.f),
	CoolingTime(3.f),
	CoolingProgress(0.f),
	OverheatSystemIntegrityDamage(50.f)
{
	ServersToRestore.Reserve(NumberOfServersToRestore);
	SetIsTimeBased(false);
}

void AObjectiveRestoreServers::BeginPlay()
{
	Super::BeginPlay();
	InitializeServerHall();
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
	if (GetIsOverheated())
	{
		WeakenSystemIntegrity(OverheatSystemIntegrityDamage * DeltaTime);
	}
	if (GetIsCooling())
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

void AObjectiveRestoreServers::InitializeServerHall()
{
	FindAllServers();
	BindControlPanel();
	InitializeTimer();
	BindPlayerLocationDetection();
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
	
	FPerformDelegate RestoredDelegate;
	RestoredDelegate.AddUObject(this, &AObjectiveRestoreServers::RegisterServerRestored);
	FServerHeatUpDelegate HeatUpDelegate;
	HeatUpDelegate.BindUObject(this, &AObjectiveRestoreServers::AddHeatBuildup);
	
	for (AObjectiveServer* Server : ServersToRestore)
	{
		Server->SetCanInteractWith(true);
		Server->SetServerState(EServerState::NeedRestoring);
		Server->SetInteractFunction(RestoredDelegate);
		Server->SetHeatUpFunction(HeatUpDelegate);
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
		FPerformDelegate CoolingCycleDelegate;
		CoolingCycleDelegate.AddUObject(this, &AObjectiveRestoreServers::RegisterControlPanelInteraction);
		ControlPanel->SetInteractFunction(CoolingCycleDelegate);
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

void AObjectiveRestoreServers::BindPlayerLocationDetection()
{
	if (PlayerLocationDetection)
	{
		FOnTriggerEnterDelegate EnterDelegate;
		EnterDelegate.AddUObject(this, &AObjectiveRestoreServers::OnEnterRoom);
		PlayerLocationDetection->SetOnTriggerEnter(EnterDelegate);

		FOnTriggerExitDelegate ExitDelegate;
		ExitDelegate.AddUObject(this, &AObjectiveRestoreServers::OnExitRoom);
		PlayerLocationDetection->SetOnTriggerExit(ExitDelegate);
	}
}

void AObjectiveRestoreServers::OnEnterRoom(APlayerLocationDetection* Room)
{
	UE_LOG(LogTemp, Display, TEXT("Entered server room"));
	StartObjective();
}

void AObjectiveRestoreServers::OnExitRoom(APlayerLocationDetection* Room)
{
	UE_LOG(LogTemp, Display, TEXT("Exited server room PlayerLocationDetection"));
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

void AObjectiveRestoreServers::RegisterControlPanelInteraction(AInteractableObject* InteractableObject)
{
	if (!GetIsActive())
	{
		return;
	}
	if (bCanOverheat)
	{
		InitiateCoolingCycle();
	}
}

void AObjectiveRestoreServers::ResetObjective()
{
	Super::ResetObjective();
	SetServerHallStatus(EServerHallStatus::Operating);
	ResetHeatBuildup();
	ActivateControlPanel(true);
	CoolingProgress = 0;
	CoolingTimer->Reset();
	ResetServersToRestore();
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
		SetServerHallStatus(EServerHallStatus::Cooling);
	}else
	{
		ControlPanel->SetCanInteractWith(true);
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
		ResetCoolingTimerProgress();
		ActivateControlPanel(true);

		for (AObjectiveServer* Server : ServersToRestore)
		{
			if (ValidServerToRestore(Server) && Server->GetIsPaused())
			{
				Server->ResumeRestoration();
			}
		}
	}
}

void AObjectiveRestoreServers::InitializeTimer()
{
	CoolingTimer = MakeUnique<FProgressTimer>(CoolingTime);

	FTimerCompletionDelegate CoolingDelegate;
	CoolingDelegate.BindUObject(this, &AObjectiveRestoreServers::ResumeOperating);
	CoolingTimer->SetCompletionDelegate(CoolingDelegate);
}

void AObjectiveRestoreServers::ResetCoolingTimerProgress() const
{
	CoolingTimer->Reset();
}

void AObjectiveRestoreServers::ResetHeatBuildup()
{
	CurrentHeatBuildup = 0.f;
}