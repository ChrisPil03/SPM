// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveDownloadStation.h"
#include "DrawDebugHelpers.h"
#include "ObjectiveManager.h"
#include "SphereTriggerComponent.h"
#include "Blueprint/UserWidget.h"

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

	if (bObjectiveComplete)
	{
		return;
	}
	
	if (bAbortObjective)
	{
		RegressObjective(DeltaTime);
	}
	else if (bObjectiveInProgress)
	{
		ProgressObjective(DeltaTime);
	}
}

void AObjectiveDownloadStation::Interact(AActor* Interactor)
{
	if(!bObjectiveInProgress && !bObjectiveComplete)
	{
		StartObjective();
	}
}

void AObjectiveDownloadStation::StartObjective()
{
	// Debugging
	UE_LOG(LogTemp, Warning, TEXT("Objective started"));
	//DrawDebugSphere(GetWorld(),
	//	GetActorLocation(),
	//	ObjectiveRadius,
	//	30,
	//	FColor::Blue,
	//	true);
	
	bObjectiveInProgress = true;
	SpawnDownloadZone();
	DisplayObjective();
}

void AObjectiveDownloadStation::AbortObjective()
{
	if (bObjectiveInProgress)
	{
		bAbortObjective = true;
	}
}

void AObjectiveDownloadStation::ResumeObjective()
{
	if (bAbortObjective)
	{
		bAbortObjective = false;
	}
}

void AObjectiveDownloadStation::CompleteObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective Complete"));
	
	bObjectiveComplete = true;
	bObjectiveInProgress = false;
	StopDisplayObjective();
	
	if (ObjectiveManager)
	{
		ObjectiveManager->RegisterCompletedObjective();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectiveManager is null!"));
	}

}

void AObjectiveDownloadStation::ResetObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Reset Objective"));
	ObjectiveProgress = 0.f;
	bObjectiveInProgress = false;
	bAbortObjective = false;
	DestroyDownloadZone();
	StopDisplayObjective();
}

void AObjectiveDownloadStation::ProgressObjective(float DeltaTime)
{
	SetObjectiveProgress(ObjectiveProgress += DeltaTime / CompletionTime);

	if (ObjectiveProgress == 1.f)
	{
		CompleteObjective();
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("Progress Objective Completion: %f"), ObjectiveProgress);
}

void AObjectiveDownloadStation::RegressObjective(float DeltaTime)
{
	SetObjectiveProgress(ObjectiveProgress -= DeltaTime / CompletionTime);

	if (ObjectiveProgress == 0.f)
	{
		ResetObjective();
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("Regress Objective Completion: %f"), ObjectiveProgress);
}

void AObjectiveDownloadStation::SetObjectiveProgress(float NewProgress)
{
	ObjectiveProgress = FMath::Clamp(NewProgress, 0.f, 1.f);
	UpdateDownloadSize();
}