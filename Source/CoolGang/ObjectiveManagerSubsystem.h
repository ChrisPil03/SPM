// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveManagerSubsystem.generated.h"

class AObjectiveDefendGenerator;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCreateObjectiveListItem, FString, ObjectiveName, AObjectiveBase*, Objective);

class AObjectiveBase;

UCLASS(BlueprintType)
class COOLGANG_API UObjectiveManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:	
	UObjectiveManagerSubsystem();

	void ActivateRandomObjective(float MalfunctionTimer, float MalfunctionInterval, float MalfunctionDamage);
	void ActivateMainObjective();
	void RegisterCompletedObjective(AObjectiveBase* CompletedObjective);
	void RegisterFailedObjective(AObjectiveBase* FailedObjective);
	void ResetAllObjectives();
	TArray<AObjectiveBase*> GetAllSubObjectives() const;

	UFUNCTION(BlueprintPure)
	AObjectiveDefendGenerator* GetMainObjective() const;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCreateObjectiveListItem CreateObjectiveListItemDelegate;

	void CreateObjectiveUIListItem(FString ObjectiveName, AObjectiveBase* Objective);

private:
	void FindObjectivesInLevel();
	void OnWorldInitialized(const UWorld::FActorsInitializedParams& Params);
	
	UPROPERTY(VisibleAnywhere)
	int32 CompletedSubObjectives;

	UPROPERTY(VisibleAnywhere)
	int32 CompletionsMainObjective;
	
	UPROPERTY(VisibleAnywhere)
	TArray<AObjectiveBase*> SubObjectivesInLevel;

	UPROPERTY(VisibleAnywhere)
	AObjectiveDefendGenerator* MainObjective;

	UPROPERTY()
	AObjectiveBase* LastCompletedObjective;
};
