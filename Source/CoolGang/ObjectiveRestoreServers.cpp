
#include "ObjectiveRestoreServers.h"
#include "ObjectiveServer.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AObjectiveRestoreServers::AObjectiveRestoreServers() : NumberOfServers(0), NumberOfServersToRestore(3)
{
	ServersToRestore.Reserve(NumberOfServersToRestore);
	RestoredServers.Reserve(NumberOfServersToRestore);
	
	SetIsTimeBased(true);

	BoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Trigger"));
	RootComponent = BoxTrigger;
}

void AObjectiveRestoreServers::BeginPlay()
{
	Super::BeginPlay();
	
	SetupTriggerEvents();
	FindAllServers();
	SelectServersToRestore();
	PrepareServersToRestore();
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
	
	FPerformDelegate Delegate;
	Delegate.AddUObject(this, &AObjectiveRestoreServers::RegisterInteraction);
	
	for (AObjectiveServer* Server : ServersToRestore)
	{
		Server->SetCanInteractWith(true);
		Server->SetServerState(EServerState::NeedRestoring);
		Server->SetInteractFunction(Delegate);
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

void AObjectiveRestoreServers::RegisterInteraction(AInteractableObject* InteractableObject)
{
	if (AObjectiveServer* Server = Cast<AObjectiveServer>(InteractableObject))
	{
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

void AObjectiveRestoreServers::ResetObjective()
{
	Super::ResetObjective();
	ServersToRestore.Empty();
	ServersToRestore.Reserve(NumberOfServersToRestore);
	RestoredServers.Empty();
	RestoredServers.Reserve(NumberOfServersToRestore);
	SelectServersToRestore();
	PrepareServersToRestore();
}

void AObjectiveRestoreServers::CompleteObjective()
{
	Super::CompleteObjective();
}

void AObjectiveRestoreServers::IncreaseObjectiveProgress(float const DeltaTime)
{
	Super::IncreaseObjectiveProgress(DeltaTime);

	if (GetObjectiveProgress() == FProgressTimer::FullCompletion)
	{
		ResetObjective();
	}
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