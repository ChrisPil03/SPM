// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "ObjectiveTimedTasks.generated.h"

class AInteractableObject;
class AObjectiveButton;

UCLASS()
class COOLGANG_API AObjectiveTimedTasks : public AObjectiveBase
{
	GENERATED_BODY()

public:
	AObjectiveTimedTasks();
	
protected:
	virtual void BeginPlay() override;
	virtual void ResetObjective() override;
	virtual void IncreaseObjectiveProgress(float const DeltaTime) override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void RegisterInteraction(AInteractableObject* InteractableObject);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"), Category="Objective")
	TArray<AInteractableObject*> AllInteractableObjects;

	UPROPERTY(VisibleAnywhere)
	int InteractedTasks;
};
