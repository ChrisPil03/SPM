// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveBase.h"

#include "PlayerLocationDetection.h"
#include "SystemIntegrity.h"
#include "VoiceLinesAudioComponent.h"
#include "Kismet/GameplayStatics.h"


AObjectiveBase::AObjectiveBase() :
	bIsActive(false),
	ObjectiveState(EObjectiveState::NotStarted),
	ObjectiveManager(nullptr),
	ObjectiveDescription("MISSING DESCRIPTION"),
	ObjectiveTime(30.f),
	bIsTimeBased(false),
	Progress(0.f),
	SystemIntegrity(nullptr),
	BaseIntegrityDamage(100.f),
	ObjectiveFailedIntegrityChunkDamage(25000.f),
	ActivatedMessage("MALFUNCTION DETECTED"),
	StartedMessage("OBJECTIVE STARTED"),
	CompletedMessage("OBJECTIVE COMPLETED"),
	FailedMessage("OBJECTIVE FAILED"),
	InPlayerLocationDetection(nullptr),
	EnterRoomVoiceLine(nullptr),
	ObjectiveActivatedVoiceLine(nullptr),
	DisplayObjectiveMessage(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	VoiceLinesAudioComponent = CreateDefaultSubobject<UVoiceLinesAudioComponent>("Voice Line Audio Component");
}

void AObjectiveBase::BeginPlay()
{
	Super::BeginPlay();
	SetIsActive(false);
	FindObjectiveManager();
	FindSystemIntegrity();
	BindPlayerLocationDetection();
	ProgressTimer = MakeUnique<FProgressTimer>(ObjectiveTime);
}

void AObjectiveBase::SetIsActive(const bool bNewState)
{
	UE_LOG(LogEngine, Warning, TEXT("State has changed to: %d"), bNewState)
	bIsActive = bNewState;
	if (bNewState)
	{
		DisplayMessage(ActivatedMessage);
		EnqueueVoiceLineAudio(ObjectiveActivatedVoiceLine);
		// 	if (OnObjectiveActivated.IsBound())
		// 	{
		// 		UE_LOG(LogEngine, Warning, TEXT("Broadcasting ACTIVATE."))
		// 		OnObjectiveActivated.Broadcast(this);
		// 	}
	} else
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

void AObjectiveBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GetIsFailed())
	{
		WeakenSystemIntegrity(BaseIntegrityDamage * DeltaTime);
	}
	if (bIsActive)
	{
		BroadcastObjectiveIsActive();
	}

	
	// if (bIsTimeBased && GetIsInProgress())
	// {
	// 	IncreaseObjectiveProgress(DeltaTime);
	// 	//BroadcastObjectiveInProgress();
	// }
}

void AObjectiveBase::StartObjective()
{
	if (GetIsNotStarted())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Objective Started"));
		SetObjectiveState(EObjectiveState::InProgress);
		DisplayMessage(StartedMessage);
		EnqueueVoiceLineAudio(ObjectiveStartedVoiceLine);
	}
}

void AObjectiveBase::ResetObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective Reset"));
	//SetIsActive(false);
	ResetProgress();
	SetObjectiveState(EObjectiveState::NotStarted);
}

void AObjectiveBase::CompleteObjective()
{
	//UE_LOG(LogTemp, Warning, TEXT("Objective Completed"));
	SetObjectiveState(EObjectiveState::Complete);
	DisplayMessage(CompletedMessage);
	EnqueueVoiceLineAudio(ObjectiveCompletedVoiceLine);

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
		//UE_LOG(LogTemp, Warning, TEXT("Objective failed"));
		SetObjectiveState(EObjectiveState::Failed);
		SetIsActive(false);
		DisplayMessage(FailedMessage);
		EnqueueVoiceLineAudio(ObjectiveFailedVoiceLine);
		WeakenSystemIntegrity(ObjectiveFailedIntegrityChunkDamage);
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

void AObjectiveBase::ResetProgress()
{
	ProgressTimer->ResetTimer();
	Progress = ProgressTimer->GetProgress();
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

void AObjectiveBase::BroadcastObjectiveIsActive()
{
	if (OnObjectiveActivated.IsBound())
	{
		OnObjectiveActivated.Broadcast(this);
	}
}

void AObjectiveBase::BindPlayerLocationDetection()
{
	if (InPlayerLocationDetection)
	{
		InPlayerLocationDetection->AddOnTriggerEnterFunction(this, &AObjectiveBase::OnTriggerEnterRoom);
		InPlayerLocationDetection->AddOnTriggerExitFunction(this, &AObjectiveBase::OnTriggerExitRoom);
	}
}

void AObjectiveBase::OnTriggerEnterRoom(APlayerLocationDetection* Room)
{
	if (GetIsActive() && GetIsNotStarted())
	{
		EnqueueVoiceLineAudio(EnterRoomVoiceLine);
	}
}

void AObjectiveBase::OnTriggerExitRoom(APlayerLocationDetection* Room)
{
	
}

void AObjectiveBase::EnqueueVoiceLineAudio(USoundBase* VoiceLine)
{
	if (VoiceLine && VoiceLinesAudioComponent)
	{
		VoiceLinesAudioComponent->EnqueueVoiceLine(VoiceLine);
	}
}

void AObjectiveBase::DisplayMessage(const FString& Message) const
{
	UE_LOG(LogTemp, Warning, TEXT("Display message"));
	if (DisplayObjectiveMessage->IsBound())
	{
		DisplayObjectiveMessage->Broadcast(Message);
		UE_LOG(LogTemp, Warning, TEXT("Display message broadcasted"));
	}
}
