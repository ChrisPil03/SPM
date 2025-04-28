// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveBase.h"
#include "ObjectiveManager.h"
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
	FindObjectiveManager();
	FindSystemIntegrity();
	ProgressTimer = MakeUnique<FProgressTimer>(ObjectiveTime);
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
	ObjectiveManager->RegisterCompletedObjective();
	SetIsActive(false);
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
	TArray<AActor*> ObjectiveManagerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AObjectiveManager::StaticClass(), ObjectiveManagerActors);

	if (ObjectiveManagerActors.Num() > 0)
	{
		ObjectiveManager = Cast<AObjectiveManager>(ObjectiveManagerActors[0]);
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectiveBase: ObjectiveManager not found"));
	}
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

