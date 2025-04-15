// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveManager.generated.h"

UCLASS()
class COOLGANG_API AObjectiveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectiveManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void RegisterCompletedObjective();

private:
	bool GetIsObjectivesCompleted() const;
	
	UPROPERTY(VisibleAnywhere)
	int CompletedObjectives = 0;

	UPROPERTY(EditInstanceOnly)
	TArray<AActor*> ObjectivesInLevel;

	void ObjectivesCompleted();
};
