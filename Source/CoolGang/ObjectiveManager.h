// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveManager.generated.h"

class AObjectiveBase;

UCLASS()
class COOLGANG_API AObjectiveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AObjectiveManager();

protected:
	virtual void BeginPlay() override;

public:
	void ActivateRandomObjective(float MalfunctionTimer, float MalfunctionInterval, float MalfunctionDamage);
	void RegisterCompletedObjective(AObjectiveBase* CompletedObjective);
	void ResetAllObjectives();

private:
	bool GetIsObjectivesCompleted() const;
	void FindObjectivesInLevel();
	
	UPROPERTY(VisibleAnywhere)
	int CompletedObjectives = 0;

	UPROPERTY(VisibleAnywhere)
	TArray<AObjectiveBase*> ObjectivesInLevel;

	UPROPERTY()
	AObjectiveBase* LastActivatedObjective;

	UPROPERTY(EditInstanceOnly)
	AActor* Portal;

	UPROPERTY(EditInstanceOnly)
	AActor* ExtractionZone;

	void ObjectivesCompleted();
};
