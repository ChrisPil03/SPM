
#include "ObjectiveRestoreServers.h"
#include "ObjectiveServer.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AObjectiveRestoreServers::AObjectiveRestoreServers() :
	NumberOfServers(0),
	NumberOfServersToRestore(3),
	ServerHallStatus(EServerHallStatus::Idle),
	MaxHeatBuildup(100.f),
	CurrentHeatBuildup(0.f)
{
	ServersToRestore.Reserve(NumberOfServersToRestore);
	RestoredServers.Reserve(NumberOfServersToRestore);
	
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

void AObjectiveRestoreServers::TriggerOverheat()
{
	if (GetIsOverheated())
	{
		return;
	}
	SetServerHallStatus(EServerHallStatus::Overheated);

	for (AObjectiveServer* Server : ServersToRestore)
	{
		if (Server)
		{
			Server->PauseRestoration();
		}
	}
}

void AObjectiveRestoreServers::InitiateCoolingCycle()
{
	if (GetIsOverheated())
	{
		SetServerHallStatus(EServerHallStatus::Operating);
		for (AObjectiveServer* Server : ServersToRestore)
		{
			if (Server)
			{
				Server->BeginCooling();
			}
		}
	}
}

void AObjectiveRestoreServers::BindControlPanel()
{
	if (ControlPanelCoolingSystem)
	{
		FPerformDelegate CoolingCycleDelegate;
		CoolingCycleDelegate.AddUObject(this, &AObjectiveRestoreServers::RegisterControlPanelInteraction);
		ControlPanelCoolingSystem->SetInteractFunction(CoolingCycleDelegate);
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectiveRestoreServers: ControlPanel is missing"));
	}
}

void AObjectiveRestoreServers::ResetHeatBuildup()
{
	CurrentHeatBuildup = 0.f;
}

void AObjectiveRestoreServers::RegisterServerRestored(AInteractableObject* InteractableObject)
{
	if (AObjectiveServer* Server = Cast<AObjectiveServer>(InteractableObject))
	{
		if (GetIsIdle())
		{
			SetServerHallStatus(EServerHallStatus::Operating);
		}
		if (ServersToRestore.Contains(Server))
		{
			ServersToRestore.Remove(Server);
			RestoredServers.Add(Server);
		}
	}
	if (RestoredServers.Num() == NumberOfServersToRestore)
	{
		CompleteObjective();
	}
}

void AObjectiveRestoreServers::RegisterControlPanelInteraction(AInteractableObject* InteractableObject)
{
	InitiateCoolingCycle();
}

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

void AObjectiveRestoreServers::ResetObjective()
{
	Super::ResetObjective();
	ServersToRestore.Empty();
	ServersToRestore.Reserve(NumberOfServersToRestore);
	RestoredServers.Empty();
	RestoredServers.Reserve(NumberOfServersToRestore);
	SelectServersToRestore();
	PrepareServersToRestore();
	SetServerHallStatus(EServerHallStatus::Idle);
	ResetHeatBuildup();

	if (ControlPanelCoolingSystem)
	{
		ControlPanelCoolingSystem->SetCanInteractWith(true);
	}
}

void AObjectiveRestoreServers::CompleteObjective()
{
	Super::CompleteObjective();
	ResetHeatBuildup();
}

void AObjectiveRestoreServers::IncreaseObjectiveProgress(float const DeltaTime)
{
	//Super::IncreaseObjectiveProgress(DeltaTime);

	//if (GetObjectiveProgress() == FProgressTimer::FullCompletion)
	//{
	//	ResetObjective();
	//}
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

	//ResetObjective();
	UE_LOG(LogTemp, Display, TEXT("Exited server room"));
}