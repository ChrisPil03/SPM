
#include "ObjectiveRestoreServers.h"
#include "ObjectiveServer.h"
#include "ScoreManagerComponent.h"
#include "Kismet/GameplayStatics.h"

AObjectiveRestoreServers::AObjectiveRestoreServers() :
	RestoredServers(0),
	NumberOfServers(0),
	NumberOfServersToSelect(3)
	// bCanOverheat(true),
	// ServerHallStatus(EServerHallStatus::Operating),
	// MaxHeatBuildup(100.f),
	// CurrentHeatBuildup(0.f),
	// CoolingTime(3.f),
	// CoolingProgress(0.f),
	// OverheatSystemIntegrityDamage(50.f),
	// FailDelay(15.f),
	// OverheatedMessage("SERVERS OVERHEATING")
{
	SetIsTimeBased(false);
}

void AObjectiveRestoreServers::BeginPlay()
{
	Super::BeginPlay();
	SetRandomServersToSelect();
	SelectedServers.Reserve(NumberOfServersToSelect);
	InitializeServerHall();
}

void AObjectiveRestoreServers::StartObjective()
{
	if (GetIsNotStarted())
	{
		Super::StartObjective();
		PrepareSelectedServers();
	}
}

void AObjectiveRestoreServers::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaTime);
	}

	// if (GetIsComplete() || GetIsFailed())
	// {
	// 	return;
	// }
	// if (GetIsOperating())
	// {
	// 	IncreaseObjectiveProgress(DeltaTime);
	// }
	// else if (GetIsOverheated())
	// {
	// 	if (!FailDelayProgressTimer)
	// 	{
	// 		return;
	// 	}
	// 	FailDelayProgressTimer->IncreaseProgress(DeltaTime);
	// }
	// else if (GetIsCooling())
	// {
	// 	CoolDown(DeltaTime);
	// }
}

void AObjectiveRestoreServers::SetIsActive(const bool bNewState)
{
	Super::SetIsActive(bNewState);
	
	if (bNewState)
	{
		SelectServers();
		ActivateControlPanel(true);
	}
}

void AObjectiveRestoreServers::FailObjective()
{
	Super::FailObjective();
	ResetServerRoom();
}

void AObjectiveRestoreServers::InitializeServerHall()
{
	ActivateControlPanel(false);
	FindAllServers();
	BindControlPanel();
	// InitializeTimers();
}

void AObjectiveRestoreServers::SelectServers()
{
	if (NumberOfServers < NumberOfServersToSelect)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected Servers to Restore: %d out of %d"), NumberOfServersToSelect, NumberOfServers);
		return;
	}

	for (int32 i = 0; i < NumberOfServersToSelect; ++i)
	{
		int32 RandomIndex = FMath::RandRange(i, NumberOfServers - 1);
		AllServers.Swap(i, RandomIndex);
	}
	SelectedServers.Append(&AllServers[0], NumberOfServersToSelect);
}

void AObjectiveRestoreServers::PrepareSelectedServers()
{
	if (SelectedServers.Num() <= 0)
	{
		return;
	}
	
	// FServerHeatUpDelegate HeatUpDelegate;
	// HeatUpDelegate.BindUObject(this, &AObjectiveRestoreServers::AddHeatBuildup);
	
	for (AObjectiveServer* Server : SelectedServers)
	{
		// Server->SetCanInteractWith(false);
		// Server->SetServerState(EServerState::NeedRestoring);
		Server->SetOnInteractFunction(this, &AObjectiveRestoreServers::OnInteract);
		// Server->SetHeatUpFunction(HeatUpDelegate);
		if (!Server->CompleteDelegate.IsAlreadyBound(this, &AObjectiveRestoreServers::RegisterServerRestored))
		{
			Server->CompleteDelegate.AddDynamic(this, &AObjectiveRestoreServers::RegisterServerRestored);
		}
		Server->StartRestoration();
	}
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
			// Server->SetCanInteractWith(false);
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

void AObjectiveRestoreServers::ResetSelectedServers()
{
	for (AObjectiveServer* Server : SelectedServers)
	{
		Server->ResetServer();
	}
	SelectedServers.Empty();
	SetRandomServersToSelect();
	SelectedServers.Reserve(NumberOfServersToSelect);
	RestoredServers = 0;
}

void AObjectiveRestoreServers::ResetServerRoom()
{
	ActivateControlPanel(false);
	ResetSelectedServers();
	// SetServerHallStatus(EServerHallStatus::Operating);
	// ResetHeatBuildup();
	// CoolingProgress = 0;
	// CoolingTimer->ResetTimer();
	// FailDelayProgressTimer->ResetTimer();
}

void AObjectiveRestoreServers::SetRandomServersToSelect()
{
	if (ServersToSelectRange.Max > ServersToSelectRange.Min && ServersToSelectRange.Min > 0)
	{
		NumberOfServersToSelect = ServersToSelectRange.GetRandomValue();
	}
}

void AObjectiveRestoreServers::OnInteract(AInteractableObject* InteractableObject)
{
	// StartObjective();
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
		if (SelectedServers.Contains(Server))
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
	// if (!GetIsActive())
	// {
	// 	return;
	// }
	// if (GetIsNotStarted())
	// {
	// 	StartObjective();
	// }
}

void AObjectiveRestoreServers::RegisterControlPanelInteraction(AInteractableObject* InteractableObject)
{
	StartObjective();
	// if (!GetIsActive() && !GetIsOverheated())
	// {
	// 	return;
	// }
	// InitiateCoolingCycle();
}

void AObjectiveRestoreServers::ResetObjective()
{
	Super::ResetObjective();
	ResetServerRoom();
}

void AObjectiveRestoreServers::CompleteObjective()
{
	Super::CompleteObjective();
	// ResetHeatBuildup();
	SetObjectiveProgress(1.f);
	OnRequestAddScore.Broadcast(EScoreType::ObjectiveServersCompleted);
	ActivateControlPanel(false);
}

void AObjectiveRestoreServers::IncreaseObjectiveProgress(float const DeltaTime)
{
	float ObjectiveProgress = 0;
	for (AObjectiveServer* Server : SelectedServers)
	{
		ObjectiveProgress += Server->GetProgress();
	}
	ObjectiveProgress /= NumberOfServersToSelect;
	SetObjectiveProgress(ObjectiveProgress);
}

// ________________________________________________________________
// ____________________ Overheat / Cooling ________________________

// void AObjectiveRestoreServers::AddHeatBuildup(float Heat)
// {
// 	if (!bCanOverheat || GetIsOverheated() || GetIsCooling())
// 	{
// 		return;
// 	}
// 	CurrentHeatBuildup += Heat;
//
// 	if (CurrentHeatBuildup >= MaxHeatBuildup)
// 	{
// 		CurrentHeatBuildup = MaxHeatBuildup;
// 		TriggerOverheat();
// 	}
// }
//
// void AObjectiveRestoreServers::TriggerOverheat()
// {
// 	if (GetIsOverheated())
// 	{
// 		return;
// 	}
// 	UE_LOG(LogTemp, Warning, TEXT("Trigger Overheat"));
// 	SetServerHallStatus(EServerHallStatus::Overheated);
// 	ActivateControlPanel(true);
// 	DisplayMessageForSeconds(OverheatedMessage, 3.f);
// 	//EnqueueVoiceLineWithMessage(OverheatVoiceLine, OverheatedMessage);
//
// 	for (AObjectiveServer* Server : ServersToRestore)
// 	{
// 		if (ValidServerToRestore(Server))
// 		{
// 			Server->PauseRestoration();
// 			Server->SetSmokeEffectActive(true);
// 		}
// 	}
// }
//
// void AObjectiveRestoreServers::InitiateCoolingCycle()
// {
// 	if (!GetIsCooling() && GetIsInProgress())
// 	{
// 		if (FailDelayProgressTimer->GetProgress() > FProgressTimer::ZeroCompletion)
// 		{
// 			FailDelayProgressTimer->ResetTimer();
// 		}
// 		SetServerHallStatus(EServerHallStatus::Cooling);
// 	}
// }
//
// void AObjectiveRestoreServers::CoolDown(float DeltaTime)
// {
// 	CoolingTimer->IncreaseProgress(DeltaTime);
// 	CoolingProgress = CoolingTimer->GetProgress();
// 	CurrentHeatBuildup -= CoolingProgress;
// 	CurrentHeatBuildup = FMath::Clamp(CurrentHeatBuildup, 0.f, MaxHeatBuildup);
// }
//
// void AObjectiveRestoreServers::ResumeOperating()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Resume Operating"));
// 	if (GetIsCooling())
// 	{
// 		SetServerHallStatus(EServerHallStatus::Operating);
// 		ResetHeatBuildup();
// 		CoolingTimer->ResetTimer();
//
// 		for (AObjectiveServer* Server : ServersToRestore)
// 		{
// 			if (ValidServerToRestore(Server) && Server->GetIsPaused())
// 			{
// 				Server->ResumeRestoration();
// 				Server->SetSmokeEffectActive(false);
// 			}
// 		}
// 	}
// }
//
// void AObjectiveRestoreServers::InitializeTimers()
// {
// 	CoolingTimer = MakeUnique<FProgressTimer>(CoolingTime);
// 	CoolingTimer->SetCompletionFunction(this, &AObjectiveRestoreServers::ResumeOperating);
// 	FailDelayProgressTimer = MakeUnique<FProgressTimer>(FailDelay);
// 	FailDelayProgressTimer->SetCompletionFunction(this, &AObjectiveRestoreServers::FailObjective);
// }
//
// void AObjectiveRestoreServers::ResetHeatBuildup()
// {
// 	CurrentHeatBuildup = 0.f;
// }