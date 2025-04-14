// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "ObjectiveDownloadStation.generated.h"

UCLASS()
class COOLGANG_API AObjectiveDownloadStation : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectiveDownloadStation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact(AActor* Interactor) override;
	
	void AbortObjective();
	void ResumeObjective();
	bool GetIsInProgress() const;

private:
	void StartObjective();
	void ProgressObjective();
	void CompleteObjective();
	
	FTimerHandle ObjectiveTimer;
	bool ObjectiveInProgress = false;
	bool ObjectiveComplete = false;
	
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	float ObjectiveProgress = 0.f;

	UPROPERTY(EditAnywhere)
	float CompletionTime = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	float ObjectiveRadius = 600.f;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	class USphereTriggerComponent* SphereTriggerComponent;
};
