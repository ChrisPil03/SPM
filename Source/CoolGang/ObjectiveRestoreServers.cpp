// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveRestoreServers.h"
#include "ObjectiveServer.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AObjectiveRestoreServers::AObjectiveRestoreServers() : NumberOfServersToRestore(3)
{
	AllServers.Empty();
	SetIsTimeBased(true);

	BoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Trigger"));
	RootComponent = BoxTrigger;
}

void AObjectiveRestoreServers::BeginPlay()
{
	Super::BeginPlay();

	BoxTrigger->OnComponentBeginOverlap.AddDynamic(this, &AObjectiveRestoreServers::OnBoxBeginOverlap);
	BoxTrigger->OnComponentEndOverlap.AddDynamic(this, &AObjectiveRestoreServers::OnBoxEndOverlap);
	
	TArray<AActor*> ServerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ObjectiveServerClass, ServerActors);
	UE_LOG(LogTemp, Warning, TEXT("Found Actors: %d"), ServerActors.Num());

	AllServers.Empty();
	AllServers.Reserve(ServerActors.Num());
	
	for (AActor* Actor : ServerActors)
	{
		if (AObjectiveServer* Server = Cast<AObjectiveServer>(Actor))
		{
			Server->SetCanInteractWith(false);
			AllServers.Add(Server);
		}
	}

	SetServersToRestore();
}

void AObjectiveRestoreServers::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsComplete())
	{
		return;
	}

	if (GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaTime);
	}
}

void AObjectiveRestoreServers::SetServersToRestore()
{
	if (AllServers.Num() < NumberOfServersToRestore)
	{
		return;
	}
	
	for (int i = 0; i < NumberOfServersToRestore; ++i)
	{
		int32 RandomIndex = FMath::RandRange(0, AllServers.Num() - 1);
		AObjectiveServer* RandomServer = AllServers[RandomIndex];
			
		if (!ServersToRestore.Contains(RandomServer) && !RestoredServers.Contains(RandomServer))
		{
			RandomServer->SetCanInteractWith(true);
			RandomServer->SetServerState(EServerState::NeedRestoring);
			ServersToRestore.Add(RandomServer);

			FPerformDelegate Delegate;
			Delegate.AddUObject(this, &AObjectiveRestoreServers::RegisterInteraction);
			RandomServer->SetInteractFunction(Delegate);
		}
		else
		{
			--i;
		}
	}
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
	SetServersToRestore();
	
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

	UE_LOG(LogTemp, Display, TEXT("Exited server room"));
}