// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveBase.h"

#include "ObjectiveManager.h"

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
	Timer = nullptr;
}

AObjectiveBase::~AObjectiveBase()
{
	if (Timer)
	{
		delete Timer;
		Timer = nullptr;
	}
}

// Called when the game starts or when spawned
void AObjectiveBase::BeginPlay()
{
	Super::BeginPlay();
	Timer = new FProgressTimer(ObjectiveTime);
}

// Called every frame
void AObjectiveBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	if (bIsTimeBased && Timer)
	{
		Timer->IncreaseProgress(DeltaTime);
	}
}

void AObjectiveBase::DecreaseObjectiveProgress(float const DeltaTime)
{
	if (bIsTimeBased && Timer)
	{
		Timer->DecreaseProgress(DeltaTime);
	}
}

void AObjectiveBase::SetObjectiveManager(AObjectiveManager* NewManager)
{
	if (NewManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectiveBase: NewManager is nullptr"));
		return;
	}
	ObjectiveManager = NewManager;
}

