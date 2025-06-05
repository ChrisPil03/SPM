#pragma once

#include "CoreMinimal.h"
#include "FProgressTimer.h"
#include "GameFramework/Actor.h"
#include "ObjectiveManagerSubsystem.h"
#include "ScoreManagerComponent.h"
#include "ObjectiveBase.generated.h"

class ADoor;
class UVoiceLineSubsystem;
// class AGate;
class UDisplayTextMessageSubsystem;
class APlayerLocationDetection;
class ASystemIntegrity;

UENUM(BlueprintType)
enum class EObjectiveState : uint8
{
	NotStarted,
	InProgress,
	Complete,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaypointEvent, AObjectiveBase*, Objective, bool, NewState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnObjectiveActivated, AObjectiveBase*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnObjectiveDeactivated, AObjectiveBase*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnObjectiveInProgress, AObjectiveBase*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveEvent);

UCLASS(Abstract, BlueprintType)
class COOLGANG_API AObjectiveBase : public AActor
{
	GENERATED_BODY()
	
public:
	AObjectiveBase();
	
protected:
	virtual void BeginPlay() override;

	virtual void StartObjective();
	virtual void CompleteObjective();
	virtual void IncreaseObjectiveProgress(float const DeltaTime);
	virtual void DecreaseObjectiveProgress(float const DeltaTime);

	UFUNCTION()
	virtual void DamageGeneratorShield(const float Damage);

	void SetObjectiveProgress(const float NewProgress);
	void SetIsTimeBased(bool const bNewState) { bIsTimeBased = bNewState; }
	FProgressTimer& GetProgressTimer() const { return *ProgressTimer; }
	
	void EnqueueVoiceLine(USoundBase* VoiceLine, const int32 Priority) const;
	// void DisplayMessageForSeconds(const FString& Message, const float Seconds) const;

	void ResetProgress();

	UPROPERTY()
	UObjectiveManagerSubsystem* ObjectiveManager;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float ShieldBaseDamage;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float ShieldChunkDamage;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EScoreType ScoreType;

public:
	UPROPERTY(BlueprintAssignable)
	FOnWaypointEvent EnableWaypoint;
	
	template <typename T>
	void AddOnObjectiveActivatedFunction(T* Object, void (T::*Func)(AObjectiveBase*))
	{
		OnObjectiveActivated.AddUObject(Object, Func);
	}
	
	template <typename T>
	void AddOnObjectiveDeactivatedFunction(T* Object, void (T::*Func)(AObjectiveBase*))
	{
		OnObjectiveDeactivated.AddUObject(Object, Func);
	}

	template <typename T>
	void AddOnObjectiveInProgressFunction(T* Object, void (T::*Func)(AObjectiveBase*))
	{
		OnObjectiveInProgress.AddUObject(Object, Func);
	}
	
	virtual void Tick(float DeltaTime) override;
	virtual void ResetObjective();
	virtual void FailObjective();

	UFUNCTION(BlueprintPure)
	APlayerLocationDetection* GetObjectivePlayerDetection() const { return InPlayerLocationDetection; }
	
	UFUNCTION(BlueprintPure)
	int32 GetObjectiveTime() const { return ObjectiveTime; }

	UFUNCTION(BlueprintCallable)
	virtual FVector GetWaypointTargetLocation() const;

	UFUNCTION(BlueprintPure)
	FString GetObjectiveName() const { return ObjectiveName; }
	UFUNCTION(BlueprintPure)
	FString GetObjectiveInitialDescription() const { return ObjectiveInitialDescription; }
	UFUNCTION(BlueprintPure)
	FString GetObjectiveStartedDescription() const { return ObjectiveStartedDescription; }

	UFUNCTION(BlueprintPure)
	virtual TArray<FString> GetUniqueObjectiveProgress() const { return TArray<FString>(); }
	
	void SetObjectiveState(EObjectiveState const NewObjectiveState) { ObjectiveState = NewObjectiveState; }
	
	UFUNCTION(BlueprintCallable, Category = "Objective States")
	bool GetIsNotStarted() const { return ObjectiveState == EObjectiveState::NotStarted; }
	UFUNCTION(BlueprintCallable, Category = "Objective States")
	bool GetIsInProgress() const { return ObjectiveState == EObjectiveState::InProgress; }
	UFUNCTION(BlueprintCallable, Category = "Objective States")
	bool GetIsComplete() const { return ObjectiveState == EObjectiveState::Complete; }
	UFUNCTION(BlueprintCallable, Category = "Objective States")
	bool GetIsFailed() const { return ObjectiveState == EObjectiveState::Failed; }
	
	UFUNCTION(BlueprintCallable, Category = "Active")
	bool GetIsActive() const { return bIsActive; }

	UFUNCTION(BlueprintCallable, Category = "Active")
	virtual void SetIsActive(const bool bNewState);

	// UFUNCTION(BlueprintCallable, Category = "Active")
	// virtual void StartMalfunctionTimer(const float MalfunctionTimer, const float MalfunctionDamageInterval, const float MalfunctionDamage);
	//
	// UFUNCTION(BlueprintCallable, Category = "Active")
	// virtual void StopMalfunctioning();
	
	UFUNCTION(BlueprintCallable, Category = "Progress")
	virtual float GetObjectiveProgress() const { return Progress; }

	UFUNCTION(BlueprintCallable, Category = "Message")
	const FString& GetActivatedMessage() const { return ActivatedMessage; }
	UFUNCTION(BlueprintCallable, Category = "Message")
	const FString& GetStartedMessage() const { return StartedMessage; }
	UFUNCTION(BlueprintCallable, Category = "Message")
	const FString& GetCompletedMessage() const { return CompletedMessage; }
	UFUNCTION(BlueprintCallable, Category = "Message")
	const FString& GetFailedMessage() const { return FailedMessage; }

	// UPROPERTY(BlueprintAssignable, Category = "Events")
	// FOnObjectiveEvent OnStartMalfunctioning;
	//
	// UPROPERTY(BlueprintAssignable, Category = "Events")
	// FOnObjectiveEvent OnStopMalfunctioning;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveEvent OnEnterObjectiveRoom;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveEvent OnExitObjectiveRoom;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveEvent OnObjectiveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveEvent OnUniqueProgressChanged;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* ObjectiveFailSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* ObjectiveStartSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* ObjectiveCompletedSound;
	

private:
	void FindObjectiveManager();
	void FindAnnouncementSubsystem();
	// void FindDisplayTextMessageSubsystem();
	void BroadcastObjectiveInProgress();
	void BroadcastObjectiveIsActive();
	void BindPlayerLocationDetection();
	void OnTriggerEnterRoom(APlayerLocationDetection* Room);
	void OnTriggerExitRoom(APlayerLocationDetection* Room);

	void StartDamageShield();
	void StopDamageShield();
	void BaseDamageGeneratorShield();

	void CloseDoors();
	void OpenDoors();
	
	FTimerHandle DamageShieldTimerHandle;

	// FTimerHandle MalfunctionTimerHandle;
	// FTimerDelegate MalfunctionTimerDelegate;
	//
	// FTimerHandle MalfunctionIntervalHandle;
	// FTimerDelegate MalfunctionIntervalDelegate;
	
	// UFUNCTION()
	// virtual void StartMalfunctioning(const float MalfunctionDamageInterval, const float MalfunctionDamage);
	
	UPROPERTY(VisibleAnywhere, Category = "Objective")
	bool bIsActive;
	
	UPROPERTY(VisibleAnywhere, Category = "Objective")
	EObjectiveState ObjectiveState;

	UPROPERTY(EditAnywhere, Category = "Objective")
	FString ObjectiveName;

	UPROPERTY(EditAnywhere, Category = "Objective")
	FString ObjectiveInitialDescription;

	UPROPERTY(EditAnywhere, Category = "Objective")
	FString ObjectiveStartedDescription;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Objective")
	int32 ObjectiveTime;
	
	bool bIsTimeBased;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	float Progress;

	UPROPERTY(EditAnywhere, Category = "Message")
	FString ActivatedMessage;

	UPROPERTY(EditAnywhere, Category = "Message")
	FString StartedMessage;

	UPROPERTY(EditAnywhere, Category = "Message")
	FString CompletedMessage;

	UPROPERTY(EditAnywhere, Category = "Message")
	FString FailedMessage;

	UPROPERTY(EditInstanceOnly, Category = "Objective")
	APlayerLocationDetection* InPlayerLocationDetection;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* EnterRoomVoiceLine;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* ObjectiveActivatedVoiceLine;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* ObjectiveStartedVoiceLine;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* ObjectiveCompletedVoiceLine;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* ObjectiveFailedVoiceLine;

	UPROPERTY()
	UVoiceLineSubsystem* VoiceLineSubsystem;

	// UPROPERTY()
	// UDisplayTextMessageSubsystem* DisplayTextMessageSubsystem;
	
	TUniquePtr<FProgressTimer> ProgressTimer;

	FOnObjectiveActivated OnObjectiveActivated;
	FOnObjectiveDeactivated OnObjectiveDeactivated;
	FOnObjectiveInProgress OnObjectiveInProgress;

	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Events")
	FOnObjectiveEvent OnObjectiveCompleted;

	// UPROPERTY(EditInstanceOnly, Category = "Room")
	// AGate* RoomGate;

	UPROPERTY(EditInstanceOnly, Category = "Room")
	TArray<ADoor*> Doors;

	UPROPERTY(VisibleAnywhere, Category = "Room")
	bool bPlayerInRoom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* IconMaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float ShieldDamageInterval;
};
