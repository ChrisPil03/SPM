// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveBase.h"
#include "ObjectiveManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AObjectiveBase::AObjectiveBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ObjectiveState = EObjectiveState::NotStarted;
	ObjectiveManager = nullptr;
	ObjectiveDescription = "Missing Description";
	ObjectiveTime = 30.f;
	bIsTimeBased = false;
}

// Called when the game starts or when spawned
void AObjectiveBase::BeginPlay()
{
	Super::BeginPlay();
	FindObjectiveManager();
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

// Called every frame
void AObjectiveBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsTimeBased && GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaTime);
	}
}

void AObjectiveBase::StartObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective Started"));
	
	SetObjectiveState(EObjectiveState::InProgress);
	DisplayObjectiveDescription();
}

void AObjectiveBase::ResetObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective Reset"));
	
	ResetProgress();
	SetObjectiveState(EObjectiveState::NotStarted);
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

void AObjectiveBase::SetObjectiveManager(AObjectiveManager* NewManager)
{
	if (!NewManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectiveBase: NewManager is nullptr"));
		return;
	}
	ObjectiveManager = NewManager;
}

void AObjectiveBase::FindObjectiveManager()
{
	TArray<AActor*> ObjectiveManagerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AObjectiveManager::StaticClass(), ObjectiveManagerActors);

	if (ObjectiveManagerActors.Num() > 0)
	{
		SetObjectiveManager(ObjectiveManager = Cast<AObjectiveManager>(ObjectiveManagerActors[0]));
	}
}

