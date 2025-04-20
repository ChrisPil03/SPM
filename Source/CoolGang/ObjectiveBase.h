// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FProgressTimer.h"
#include "GameFramework/Actor.h"
#include "ObjectiveBase.generated.h"

class AObjectiveManager;

UENUM(BlueprintType)
enum class EObjectiveState : uint8
{
	NotStarted,
	InProgress,
	Aborting,
	Complete
};

UCLASS(Abstract)
class COOLGANG_API AObjectiveBase : public AActor
{
	GENERATED_BODY()
	
public:
	AObjectiveBase();
	virtual ~AObjectiveBase() override;
	
protected:
	virtual void BeginPlay() override;

	virtual void StartObjective();
	virtual void ResetObjective();
	virtual void CompleteObjective();
	virtual void IncreaseObjectiveProgress(float const DeltaTime);
	virtual void DecreaseObjectiveProgress(float const DeltaTime);
	
	void SetIsTimeBased(bool const bNewState) { bIsTimeBased = bNewState; }

	UFUNCTION(BlueprintCallable)
	float GetObjectiveProgress() const { return Timer->GetProgress(); }

public:	
	virtual void Tick(float DeltaTime) override;
	
	void SetObjectiveManager(AObjectiveManager* NewManager);

	void SetObjectiveState(EObjectiveState const NewObjectiveState) { ObjectiveState = NewObjectiveState; }
	bool GetIsNotStarted() const { return ObjectiveState == EObjectiveState::NotStarted; }
	bool GetIsInProgress() const { return ObjectiveState == EObjectiveState::InProgress; }
	bool GetIsAborting() const { return ObjectiveState == EObjectiveState::Aborting; }
	bool GetIsComplete() const { return ObjectiveState == EObjectiveState::Complete; }
	
	UFUNCTION(BlueprintImplementableEvent)
	void DisplayObjectiveDescription();

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayObjectiveCompleted();

	UFUNCTION(BlueprintImplementableEvent)
	void StopDisplayObjectiveDescription();

private:
	void ResetProgress() const { Timer->Reset(); }
	
	UPROPERTY(VisibleAnywhere, Category = "Objective")
	EObjectiveState ObjectiveState;

	UPROPERTY(EditAnywhere)
	AObjectiveManager* ObjectiveManager;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Objective")
	float ObjectiveTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Objective")
	FString ObjectiveDescription;
	
	bool bIsTimeBased;
	
	FProgressTimer* Timer;
};
