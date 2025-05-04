// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCreateObjectiveListItem, FString, ObjectiveName, AObjectiveBase*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDisplayObjectiveMessage, FString, Message);

class AObjectiveBase;

UCLASS(BlueprintType)
class COOLGANG_API UObjectiveManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:	
	UObjectiveManagerSubsystem();

	void ActivateRandomObjective(float MalfunctionTimer, float MalfunctionInterval, float MalfunctionDamage);
	void RegisterCompletedObjective(AObjectiveBase* CompletedObjective);
	void ResetAllObjectives();
	TArray<AObjectiveBase*> GetAllObjectives() const;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCreateObjectiveListItem CreateObjectiveListItemDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDisplayObjectiveMessage DisplayObjectiveMessage;

private:
	void FindObjectivesInLevel();
	void OnWorldInitialized(const UWorld::FActorsInitializedParams& Params);
	void CreateObjectiveUIListItem(FString ObjectiveName, AObjectiveBase* Objective);
	void BindDisplayMessage(AObjectiveBase* Objective);
	
	UPROPERTY(VisibleAnywhere)
	int CompletedObjectives = 0;

	UPROPERTY(VisibleAnywhere)
	TArray<AObjectiveBase*> ObjectivesInLevel;

	UPROPERTY()
	AObjectiveBase* LastActivatedObjective;
};
