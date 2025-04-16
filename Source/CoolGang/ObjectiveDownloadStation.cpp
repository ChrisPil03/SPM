// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveDownloadStation.h"
#include "DrawDebugHelpers.h"
#include "ObjectiveManager.h"
#include "SphereTriggerComponent.h"
#include "TimerManager.h"

// Sets default values
AObjectiveDownloadStation::AObjectiveDownloadStation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereTriggerComponent = CreateDefaultSubobject<USphereTriggerComponent>(TEXT("Sphere Trigger Component"));
}

// Called when the game starts or when spawned
void AObjectiveDownloadStation::BeginPlay()
{
	Super::BeginPlay();
	SphereTriggerComponent->SetSphereRadius(ObjectiveRadius);
}

// Called every frame
void AObjectiveDownloadStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ObjectiveInProgress)
	{
		ProgressObjective();
	}
}

void AObjectiveDownloadStation::Interact(AActor* Interactor)
{
	if(!ObjectiveInProgress && !ObjectiveComplete)
	{
		StartObjective();
	}
}

bool AObjectiveDownloadStation::GetIsInProgress() const
{
	return ObjectiveInProgress;
}

void AObjectiveDownloadStation::StartObjective()
{
	// Debugging
	//UE_LOG(LogTemp, Warning, TEXT("Interacted with objective"));
	DrawDebugSphere(GetWorld(), GetActorLocation(), ObjectiveRadius, 30, FColor::Blue, true);
	
	ObjectiveInProgress = true;
	SpawnDownloadZone();
	GetWorldTimerManager().SetTimer(ObjectiveTimer, this, &AObjectiveDownloadStation::CompleteObjective, CompletionTime, false);
}

void AObjectiveDownloadStation::ProgressObjective()
{
	ObjectiveProgress = GetWorldTimerManager().GetTimerElapsed(ObjectiveTimer) / CompletionTime;
	UpdateDownloadSize();
	
	UE_LOG(LogTemp, Warning, TEXT("Objective Completion: %f"), ObjectiveProgress);
}

void AObjectiveDownloadStation::AbortObjective()
{
	GetWorldTimerManager().PauseTimer(ObjectiveTimer);
}

void AObjectiveDownloadStation::ResumeObjective()
{
	GetWorldTimerManager().UnPauseTimer(ObjectiveTimer);
}

void AObjectiveDownloadStation::CompleteObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective Complete"));

	ObjectiveProgress = 1.f;
	ObjectiveComplete = true;
	ObjectiveInProgress = false;
	ObjectiveManager->RegisterCompletedObjective();
}
