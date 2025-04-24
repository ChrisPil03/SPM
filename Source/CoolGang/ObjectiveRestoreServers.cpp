
#include "ObjectiveRestoreServers.h"
#include "ObjectiveServer.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AObjectiveRestoreServers::AObjectiveRestoreServers() :
	NumberOfServers(0),
	NumberOfServersToRestore(3),
	ServerHallStatus(EServerHallStatus::Operating),
	MaxHeatBuildup(100.f),
	CurrentHeatBuildup(0.f),
	CoolingTime(3.f),
	CoolingProgress(0.f)
{
	ServersToRestore.Reserve(NumberOfServersToRestore);
	
	SetIsTimeBased(false);

	BoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Trigger"));
	RootComponent = BoxTrigger;
}

void AObjectiveRestoreServers::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsComplete())
	{
		return;
	}

	if (GetIsCooling())
	{
		CoolDown(DeltaTime);
	}

	if (GetIsOperating())
	{
		IncreaseObjectiveProgress(DeltaTime);
	}
}

void AObjectiveRestoreServers::BeginPlay()
{
	Super::BeginPlay();
	InitializeServerHall();
}

void AObjectiveRestoreServers::InitializeServerHall()
{
	SetupTriggerEvents();
	FindAllServers();
	SelectServersToRestore();
	PrepareServersToRestore();
	BindControlPanel();
	InitializeTimer();
}

void AObjectiveRestoreServers::SelectServersToRestore()
{
	if (NumberOfServers < NumberOfServersToRestore)
	{
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
}

void AObjectiveRestoreServers::SetupTriggerEvents()
{
	BoxTrigger->OnComponentBeginOverlap.AddDynamic(this, &AObjectiveRestoreServers::OnBoxBeginOverlap);
	BoxTrigger->OnComponentEndOverlap.AddDynamic(this, &AObjectiveRestoreServers::OnBoxEndOverlap);
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

void AObjectiveRestoreServers::ActivateControlPanel(const bool NewState)
{
	if (ControlPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Set can interact with control panel"));
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
	InitiateCoolingCycle();
}

void AObjectiveRestoreServers::ResetObjective()
{
	Super::ResetObjective();
	ServersToRestore.Empty();
	ServersToRestore.Reserve(NumberOfServersToRestore);
	SelectServersToRestore();
	PrepareServersToRestore();
	SetServerHallStatus(EServerHallStatus::Operating);
	ResetHeatBuildup();
	ActivateControlPanel(true);
}

void AObjectiveRestoreServers::CompleteObjective()
{
	Super::CompleteObjective();
	ResetHeatBuildup();
	SetObjectiveProgress(1.f);
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

void AObjectiveRestoreServers::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (GetIsNotStarted())
	{
		StartObjective();
	}
}

void AObjectiveRestoreServers::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	
	UE_LOG(LogTemp, Display, TEXT("Exited server room"));
}

// ________________________________________________________________
// ____________________ Overheat / Cooling ________________________

void AObjectiveRestoreServers::AddHeatBuildup(float Heat)
{
	if (GetIsOverheated())
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
	if (GetIsOverheated() && !GetIsCooling())
	{
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
	UE_LOG(LogTemp, Warning, TEXT("Resume"))
	if (GetIsCooling())
	{
		SetServerHallStatus(EServerHallStatus::Operating);
		ResetHeatBuildup();
		ResetCoolingTimerProgress();
		ActivateControlPanel(true);

		for (AObjectiveServer* Server : ServersToRestore)
		{
			if (ValidServerToRestore(Server))
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