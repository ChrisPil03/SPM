#include "ObjectiveBase.h"

#include "DisplayTextMessageSubsystem.h"
#include "PlayerLocationDetection.h"
#include "SystemIntegrity.h"
#include "AnnouncementSubsystem.h"
#include "Gate.h"
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
	ObjectiveStartedVoiceLine(nullptr),
	ObjectiveCompletedVoiceLine(nullptr),
	ObjectiveFailedVoiceLine(nullptr),
	AnnouncementSubsystem(nullptr),
	DisplayTextMessageSubsystem(nullptr),
	RoomGate(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AObjectiveBase::BeginPlay()
{
	Super::BeginPlay();
	SetIsActive(false);
	FindObjectiveManager();
	FindAnnouncementSubsystem();
	FindDisplayTextMessageSubsystem();
	FindSystemIntegrity();
	BindPlayerLocationDetection();
	ProgressTimer = MakeUnique<FProgressTimer>(ObjectiveTime);
}

void AObjectiveBase::SetIsActive(const bool bNewState)
{
	//UE_LOG(LogEngine, Warning, TEXT("State has changed to: %d"), bNewState)
	bIsActive = bNewState;
	if (bNewState)
	{
		if (RoomGate)
		{
			RoomGate->OpenGate();
		}
		DisplayMessageForSeconds(ActivatedMessage, 3.f);
		//EnqueueVoiceLineWithMessage(ObjectiveActivatedVoiceLine, ActivatedMessage);
		
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
	MalfunctionTimerDelegate.BindUFunction(this, FName("StartMalfunctioning"), MalfunctionDamageInterval, MalfunctionDamage);
	GetWorldTimerManager().SetTimer(MalfunctionTimerHandle, MalfunctionTimerDelegate, MalfunctionTimer, false);

}

void AObjectiveBase::StopMalfunctioning()
{
	GetWorldTimerManager().ClearTimer(MalfunctionTimerHandle);
	GetWorldTimerManager().ClearTimer(MalfunctionIntervalHandle);

	if (OnStopWeakeningSystemIntegrity.IsBound())
	{
		OnStopWeakeningSystemIntegrity.Broadcast();
	}
}

void AObjectiveBase::StartMalfunctioning(const float MalfunctionDamageInterval, const float MalfunctionDamage)
{
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
		SetObjectiveState(EObjectiveState::InProgress);
		DisplayMessageForSeconds(StartedMessage, 3.f);
		StopMalfunctioning();
		//EnqueueVoiceLineWithMessage(ObjectiveStartedVoiceLine, StartedMessage);
	}
}

void AObjectiveBase::ResetObjective()
{
	UE_LOG(LogTemp, Warning, TEXT("Objective Reset"));
	ResetProgress();
	SetObjectiveState(EObjectiveState::NotStarted);
}

void AObjectiveBase::CompleteObjective()
{
	SetObjectiveState(EObjectiveState::Complete);
	DisplayMessageForSeconds(CompletedMessage, 3.f);
	//EnqueueVoiceLineWithMessage(ObjectiveCompletedVoiceLine, CompletedMessage);
	if (OnObjectiveCompleted.IsBound())
	{
		OnObjectiveCompleted.Broadcast();
	}

	if (ObjectiveManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectiveBase: ObjectiveManager is nullptr"));
		return;
	}
	ObjectiveManager->RegisterCompletedObjective(this);

	if (!bPlayerInRoom)
	{
		RoomGate->CloseGate();
	}
}

void AObjectiveBase::FailObjective()
{
	if (!GetIsFailed())
	{
		SetObjectiveState(EObjectiveState::Failed);
		SetIsActive(false);
		DisplayMessageForSeconds(FailedMessage, 3.f);
		//EnqueueVoiceLineWithMessage(ObjectiveFailedVoiceLine, FailedMessage);
		WeakenSystemIntegrity(ObjectiveFailedIntegrityChunkDamage);

		if (!bPlayerInRoom)
		{
			RoomGate->CloseGate();
		}
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
	if (SystemIntegrity && GetIsNotStarted() || GetIsFailed())
	{
		SystemIntegrity->WeakenIntegrity(Damage);

		if (OnWeakenSystemIntegrity.IsBound())
		{
			OnWeakenSystemIntegrity.Broadcast();
		}
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

void AObjectiveBase::FindAnnouncementSubsystem()
{
	AnnouncementSubsystem = GetGameInstance()->GetSubsystem<UAnnouncementSubsystem>();
}

void AObjectiveBase::FindDisplayTextMessageSubsystem()
{
	DisplayTextMessageSubsystem = GetWorld()->GetSubsystem<UDisplayTextMessageSubsystem>();
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
	bPlayerInRoom = true;
	
	if (GetIsActive() && GetIsNotStarted())
	{
		//EnqueueVoiceLineWithMessage(EnterRoomVoiceLine, "");
	}
}

void AObjectiveBase::OnTriggerExitRoom(APlayerLocationDetection* Room)
{
	bPlayerInRoom = false;
	
	if (!GetIsActive())
	{
		if (RoomGate)
		{
			UE_LOG(LogTemp, Warning, TEXT("Close the gate"));
			RoomGate->CloseGate();	
		}
	}
}

void AObjectiveBase::EnqueueVoiceLineWithMessage(USoundBase* VoiceLine, const FString& Message) const
{
	if (VoiceLine && AnnouncementSubsystem)
	{
		AnnouncementSubsystem->EnqueueVoiceLineWithMessage(VoiceLine, Message);
	}
}

void AObjectiveBase::DisplayMessageForSeconds(const FString& Message, const float Seconds) const
{
	if (DisplayTextMessageSubsystem)
	{
		DisplayTextMessageSubsystem->DisplayMessageForSeconds(Message, Seconds);
	}
}