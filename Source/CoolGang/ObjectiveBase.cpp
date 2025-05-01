// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveBase.h"
#include "ObjectiveManagerSubsystem.h"
#include "SystemIntegrity.h"
#include "Kismet/GameplayStatics.h"


AObjectiveBase::AObjectiveBase() :
	bIsActive(false),
	ObjectiveState(EObjectiveState::NotStarted),
	ObjectiveManager(nullptr),
	ObjectiveDescription("Missing Description"),
	ObjectiveTime(30.f),
	bIsTimeBased(false),
	Progress(0.f),
	SystemIntegrity(nullptr),
	BaseIntegrityDamage(100.f)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AObjectiveBase::BeginPlay()
{
	Super::BeginPlay();
	SetIsActive(false);
	FindObjectiveManager();
	FindSystemIntegrity();
	ProgressTimer = MakeUnique<FProgressTimer>(ObjectiveTime);
}

void AObjectiveBase::SetIsActive(const bool bNewState)
{
	UE_LOG(LogEngine, Warning, TEXT("State has changed to: %d"), bNewState)
	bIsActive = bNewState;
	if (bNewState)
	{
		if (OnObjectiveActivated.IsBound())
		{
			UE_LOG(LogEngine, Warning, TEXT("Broadcasting ACTIVATE."))
			OnObjectiveActivated.Broadcast(this);
		}
	}
	else
	{
		if (OnObjectiveDeactivated.IsBound())
		{
			UE_LOG(LogEngine, Warning, TEXT("Broadcasting DEACTIVATE."))
			OnObjectiveDeactivated.Broadcast(this);
		}
	}
	
}

void AObjectiveBase::StartMalfunctionTimer(const float MalfunctionTimer, const float MalfunctionDamageInterval, const float MalfunctionDamage)
{
	UE_LOG(LogTemp, Warning, TEXT("Starting Malfunction Timer"));
	MalfunctionTimerDelegate.BindUFunction(this, FName("StartMalfunctioning"), MalfunctionDamageInterval, MalfunctionDamage);
	GetWorldTimerManager().SetTimer(MalfunctionTimerHandle, MalfunctionTimerDelegate, MalfunctionTimer, false);

}

void AObjectiveBase::StopMalfunctioning()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective complete, stopping malfunction"));
	GetWorldTimerManager().ClearTimer(MalfunctionTimerHandle);
	GetWorldTimerManager().ClearTimer(MalfunctionIntervalHandle);
}

void AObjectiveBase::StartMalfunctioning(const float MalfunctionDamageInterval, const float MalfunctionDamage)
{
	UE_LOG(LogTemp, Warning, TEXT("Starting to malfunction"));
	MalfunctionIntervalDelegate.BindUFunction(this, FName("WeakenSystemIntegrity"), MalfunctionDamage);
	GetWorldTimerManager().SetTimer(MalfunctionIntervalHandle, MalfunctionIntervalDelegate, MalfunctionDamageInterval, true);
}

float AObjectiveBase::GetObjectiveProgress() const
{
	if (!ProgressTimer)
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectiveBase: Timer is nullptr, cannot get progress"));
	}
	return ProgressTimer->GetProgress();
}

void AObjectiveBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GetIsFailed())
	{
		WeakenSystemIntegrity(BaseIntegrityDamage * DeltaTime);
	}
	if (!bIsActive)
	{
		return;
	}
	if (bIsTimeBased && GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaTime);
		BroadcastObjectiveInProgress();
	}
}

void AObjectiveBase::StartObjective()
{
	if (GetIsNotStarted())
	{
		UE_LOG(LogTemp, Warning, TEXT("Objective Started"));
		SetObjectiveState(EObjectiveState::InProgress);
		DisplayObjectiveDescription();
	}
}

void AObjectiveBase::ResetObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective Reset"));

	SetIsActive(false);
	SetObjectiveState(EObjectiveState::NotStarted);
	ResetProgress();
	StopDisplayObjectiveDescription();
}

void AObjectiveBase::CompleteObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective Completed"));
	
	SetObjectiveState(EObjectiveState::Complete);
	StopDisplayObjectiveDescription();
	DisplayObjectiveCompleted();

	if (ObjectiveManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectiveBase: ObjectiveManager is nullptr"));
		return;
	}
	ObjectiveManager->RegisterCompletedObjective(this);
}

void AObjectiveBase::FailObjective()
{
	if (!GetIsFailed())
	{
		UE_LOG(LogTemp, Warning, TEXT("Objective failed"));
		SetObjectiveState(EObjectiveState::Failed);
		SetIsActive(false);
	}
}

void AObjectiveBase::IncreaseObjectiveProgress(float const DeltaTime)
{
	if (bIsTimeBased && ProgressTimer)
	{
		ProgressTimer->IncreaseProgress(DeltaTime);
	}
	Progress = ProgressTimer->GetProgress();
}

void AObjectiveBase::DecreaseObjectiveProgress(float const DeltaTime)
{
	if (bIsTimeBased && ProgressTimer)
	{
		ProgressTimer->DecreaseProgress(DeltaTime);
	}
	Progress = ProgressTimer->GetProgress();
}

void AObjectiveBase::SetObjectiveProgress(const float NewProgress)
{
	Progress = NewProgress;

	if (bIsTimeBased)
	{
		ProgressTimer->SetProgress(NewProgress);
	}
}

void AObjectiveBase::WeakenSystemIntegrity(const float Damage)
{
	if (SystemIntegrity)
	{
		SystemIntegrity->WeakenIntegrity(Damage);
	}
}

void AObjectiveBase::FindObjectiveManager()
{
	ObjectiveManager = GetWorld()->GetSubsystem<UObjectiveManagerSubsystem>();
}

void AObjectiveBase::FindSystemIntegrity()
{
	TArray<AActor*> SystemIntegrityActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASystemIntegrity::StaticClass(), SystemIntegrityActors);

	if (SystemIntegrityActors.Num() > 0)
	{
		SystemIntegrity = Cast<ASystemIntegrity>(SystemIntegrityActors[0]);
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectiveBase: SystemIntegrity not found"));
	}
}

void AObjectiveBase::BroadcastObjectiveInProgress()
{
	if (OnObjectiveInProgress.IsBound())
	{
		UE_LOG(LogEngine, Warning, TEXT("Broadcasting InProgress."));
		OnObjectiveInProgress.Broadcast(this);
	}
}