// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectivePressButtonsManager.generated.h"

class AObjectiveButton;

UCLASS()
class COOLGANG_API AObjectivePressButtonsManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectivePressButtonsManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void RegisterButtonPressed();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"), Category="Objective")
	TArray<AObjectiveButton*> AllButtons;

	UPROPERTY(VisibleAnywhere)
	int ButtonsPressed = 0;
	
	FTimerHandle ObjectiveTimer;
	bool ObjectiveInProgress = false;
	bool ObjectiveComplete = false;
	
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Objective")
	float ObjectiveProgress = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"), Category="Objective")
	float TimeLimit = 15.f;
	
	void ResetObjective();
	void StartObjective();
	void ProgressObjective();
	void CompleteObjective();
	void OnTimerEnd();
};
