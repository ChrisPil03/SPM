#include "ObjectiveBase.h"
#include "DisplayTextMessageSubsystem.h"
#include "PlayerLocationDetection.h"
#include "AnnouncementSubsystem.h"
#include "Gate.h"
#include "ObjectiveDefendGenerator.h"

AObjectiveBase::AObjectiveBase() :
	bIsActive(false),
	ObjectiveState(EObjectiveState::NotStarted),
	ObjectiveManager(nullptr),
	ObjectiveName("MISSING NAME"),
	ObjectiveInitialDescription("MISSING DESCRIPTION ONE"),
	ObjectiveStartedDescription("MISSING DESCRIPTION TWO"),
	ObjectiveTime(30),
	bIsTimeBased(false),
	Progress(0.f),
	ShieldBaseDamage(100.f),
	ShieldChunkDamage(5000.f),
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
		//DisplayMessageForSeconds(ActivatedMessage, 3.f);
		EnqueueVoiceLineWithMessage(ObjectiveActivatedVoiceLine, ActivatedMessage);
		
		if (OnObjectiveActivated.IsBound())
		{
			UE_LOG(LogEngine, Warning, TEXT("Broadcasting ACTIVATE."))
			OnObjectiveActivated.Broadcast(this);
		}
		if (EnableWaypoint.IsBound())
		{
			EnableWaypoint.Broadcast(this, true);
		}
		if (bPlayerInRoom)
		{
			if (OnEnterObjectiveRoom.IsBound())
			{
				OnEnterObjectiveRoom.Broadcast();
				UE_LOG(LogTemp, Warning, TEXT("Broadcasting OnEnterRoom"))
			}
		}
	} else
	{
		if (OnObjectiveDeactivated.IsBound())
		{
			UE_LOG(LogEngine, Warning, TEXT("Broadcasting DEACTIVATE."))
			OnObjectiveDeactivated.Broadcast(this);
		}
		// StopMalfunctioning();
		if (EnableWaypoint.IsBound())
		{
			EnableWaypoint.Broadcast(this, false);
		}
		if (!bPlayerInRoom && RoomGate)
		{
			RoomGate->CloseGate();
		}
	}
}

// void AObjectiveBase::StartMalfunctionTimer(const float MalfunctionTimer, const float MalfunctionDamageInterval, const float MalfunctionDamage)
// {
// 	if (!Cast<AObjectiveDefendGenerator>(this))
// 	{
// 		MalfunctionTimerDelegate.BindUFunction(
// 			this,
// 			FName("StartMalfunctioning"),
// 			MalfunctionDamageInterval,
// 			MalfunctionDamage);
// 		GetWorldTimerManager().SetTimer(
// 			MalfunctionTimerHandle,
// 			MalfunctionTimerDelegate,
// 			MalfunctionTimer,
// 			false);
// 	}
// }

// void AObjectiveBase::StopMalfunctioning()
// {
// 	GetWorldTimerManager().ClearTimer(MalfunctionTimerHandle);
// 	GetWorldTimerManager().ClearTimer(MalfunctionIntervalHandle);
//
// 	if (OnStopMalfunctioning.IsBound())
// 	{
// 		OnStopMalfunctioning.Broadcast();
// 	}
// }
//
// void AObjectiveBase::StartMalfunctioning(const float MalfunctionDamageInterval, const float MalfunctionDamage)
// {
// 	MalfunctionIntervalDelegate.BindUFunction(this, FName("DamageGeneratorShield"), MalfunctionDamage);
// 	GetWorldTimerManager().SetTimer(MalfunctionIntervalHandle, MalfunctionIntervalDelegate, MalfunctionDamageInterval, true);
//
// 	if (OnStartMalfunctioning.IsBound())
// 	{
// 		OnStartMalfunctioning.Broadcast();
// 	}
// }

void AObjectiveBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsActive)
	{
		BroadcastObjectiveIsActive();
		DamageGeneratorShield(DeltaTime * ShieldBaseDamage);
	}
	if (bIsTimeBased && GetIsInProgress())
	{
		BroadcastObjectiveInProgress();
	}
}

void AObjectiveBase::StartObjective()
{
	if (GetIsNotStarted())
	{
		SetObjectiveState(EObjectiveState::InProgress);
		// StopMalfunctioning();
		//DisplayMessageForSeconds(StartedMessage, 3.f);
		EnqueueVoiceLineWithMessage(ObjectiveStartedVoiceLine, StartedMessage);

		if (EnableWaypoint.IsBound())
		{
			EnableWaypoint.Broadcast(this, false);
		}

		if (OnObjectiveStarted.IsBound())
		{
			OnObjectiveStarted.Broadcast();
		}
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
	//DisplayMessageForSeconds(CompletedMessage, 3.f);
	EnqueueVoiceLineWithMessage(ObjectiveCompletedVoiceLine, CompletedMessage);
	if (OnObjectiveCompleted.IsBound())
	{
		OnObjectiveCompleted.Broadcast();
	}
	if (EnableWaypoint.IsBound())
	{
		EnableWaypoint.Broadcast(this, false);
	}

	if (ObjectiveManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectiveBase: ObjectiveManager is nullptr"));
		return;
	}
	ObjectiveManager->RegisterCompletedObjective(this);

	if (!bPlayerInRoom && RoomGate)
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
		//DisplayMessageForSeconds(FailedMessage, 3.f);
		EnqueueVoiceLineWithMessage(ObjectiveFailedVoiceLine, FailedMessage);
		DamageGeneratorShield(ShieldChunkDamage);

		if (!bPlayerInRoom && RoomGate)
		{
			RoomGate->CloseGate();
		}

		if (EnableWaypoint.IsBound())
		{
			EnableWaypoint.Broadcast(this, false);
		}

		if (ObjectiveManager)
		{
			ObjectiveManager->RegisterFailedObjective(this);
		}
	}
}

FVector AObjectiveBase::GetWaypointTargetLocation() const
{
	// FVector Origin;
	// FVector Extent;
	// GetActorBounds(false, Origin, Extent, false);
	return GetActorLocation();
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

void AObjectiveBase::DamageGeneratorShield(const float Damage)
{
	if (AObjectiveDefendGenerator* Generator = ObjectiveManager->GetMainObjective())
	{
		if (Generator != this)
		{
			Generator->DamageGeneratorShield(Damage);	
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

void AObjectiveBase::BroadcastObjectiveInProgress()
{
	if (OnObjectiveInProgress.IsBound())
	{
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
	
	if (GetIsActive())
	{
		if (OnEnterObjectiveRoom.IsBound())
		{
			OnEnterObjectiveRoom.Broadcast();
		}
	}

	if (GetIsInProgress())
	{
		if (EnableWaypoint.IsBound())
		{
			EnableWaypoint.Broadcast(this, false);
		}
	}
}

void AObjectiveBase::OnTriggerExitRoom(APlayerLocationDetection* Room)
{
	bPlayerInRoom = false;
	
	if (!GetIsActive())
	{
		if (RoomGate)
		{
			RoomGate->CloseGate();	
		}
	}else
	{
		if (OnExitObjectiveRoom.IsBound())
		{
			OnExitObjectiveRoom.Broadcast();
		}
	}
	if (GetIsInProgress())
	{
		if (EnableWaypoint.IsBound())
		{
			EnableWaypoint.Broadcast(this, true);
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