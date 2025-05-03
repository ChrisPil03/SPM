// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FProgressTimer.h"
#include "GameFramework/Actor.h"
#include "ObjectiveBase.generated.h"

class ASystemIntegrity;
class UObjectiveManagerSubsystem;

UENUM(BlueprintType)
enum class EObjectiveState : uint8
{
	NotStarted,
	InProgress,
	Complete,
	Failed
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnObjectiveActivated, AObjectiveBase*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnObjectiveDeactivated, AObjectiveBase*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnObjectiveInProgress, AObjectiveBase*);

UCLASS(Abstract)
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
	virtual void WeakenSystemIntegrity(const float Damage);

	void SetObjectiveProgress(const float NewProgress);
	void SetIsTimeBased(bool const bNewState) { bIsTimeBased = bNewState; }
	FProgressTimer& GetProgressTimer() const { return *ProgressTimer; }
	ASystemIntegrity* GetSystemIntegrity() const { return SystemIntegrity; }
	float GetBaseIntegrityDamage() const { return BaseIntegrityDamage; }

public:

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

	FString GetObjectiveName() const { return ObjectiveDescription; }
	
	void SetObjectiveState(EObjectiveState const NewObjectiveState) { ObjectiveState = NewObjectiveState; }
	
	UFUNCTION(BlueprintCallable, Category = "Objective States")
	bool GetIsNotStarted() const { return ObjectiveState == EObjectiveState::NotStarted; }
	UFUNCTION(BlueprintCallable, Category = "Objective States")
	bool GetIsInProgress() const { return ObjectiveState == EObjectiveState::InProgress; }
	// UFUNCTION(BlueprintCallable, Category = "Objective States")
	// bool GetIsAborting() const { return ObjectiveState == EObjectiveState::Aborting; }
	UFUNCTION(BlueprintCallable, Category = "Objective States")
	bool GetIsComplete() const { return ObjectiveState == EObjectiveState::Complete; }
	UFUNCTION(BlueprintCallable, Category = "Objective States")
	bool GetIsFailed() const { return ObjectiveState == EObjectiveState::Failed; }
	
	UFUNCTION(BlueprintCallable, Category = "Active")
	bool GetIsActive() const { return bIsActive; }

	UFUNCTION(BlueprintCallable, Category = "Active")
	virtual void SetIsActive(const bool bNewState);

	UFUNCTION(BlueprintCallable, Category = "Active")
	virtual void StartMalfunctionTimer(const float MalfunctionTimer, const float MalfunctionDamageInterval, const float MalfunctionDamage);
	
	UFUNCTION(BlueprintCallable, Category = "Active")
	virtual void StopMalfunctioning();
	
	UFUNCTION(BlueprintCallable, Category = "Progress")
	virtual float GetObjectiveProgress() const { return Progress; }
	
	UFUNCTION(BlueprintImplementableEvent)
	void DisplayObjectiveDescription();

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayObjectiveCompleted();

	UFUNCTION(BlueprintImplementableEvent)
	void StopDisplayObjectiveDescription();

private:
	void ResetProgress() const { ProgressTimer->Reset(); }
	void FindObjectiveManager();
	void FindSystemIntegrity();
	void BroadcastObjectiveInProgress();
	void BroadcastObjectiveIsActive();

	FTimerHandle MalfunctionTimerHandle;
	FTimerDelegate MalfunctionTimerDelegate;

	FTimerHandle MalfunctionIntervalHandle;
	FTimerDelegate MalfunctionIntervalDelegate;
	
	UFUNCTION()
	virtual void StartMalfunctioning(const float MalfunctionDamageInterval, const float MalfunctionDamage);
	
	UPROPERTY(VisibleAnywhere, Category = "Objective")
	bool bIsActive;
	
	UPROPERTY(VisibleAnywhere, Category = "Objective")
	EObjectiveState ObjectiveState;

	UPROPERTY()
	UObjectiveManagerSubsystem* ObjectiveManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Objective")
	FString ObjectiveDescription;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Objective")
	float ObjectiveTime;
	
	bool bIsTimeBased;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	float Progress;

	UPROPERTY()
	ASystemIntegrity* SystemIntegrity;
	
	UPROPERTY(EditAnywhere, Category = "System Integrity")
	float BaseIntegrityDamage;

	UPROPERTY(EditAnywhere, Category = "System Integrity")
	float ObjectiveFailedIntegrityChunkDamage;
	
	TUniquePtr<FProgressTimer> ProgressTimer;
	
	FOnObjectiveActivated OnObjectiveActivated;
	FOnObjectiveDeactivated OnObjectiveDeactivated;
	FOnObjectiveInProgress OnObjectiveInProgress;
};
