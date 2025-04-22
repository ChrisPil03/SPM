// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "ObjectiveRestoreServers.generated.h"

class UBoxComponent;
class AInteractableObject;
class AObjectiveServer;

UCLASS()
class COOLGANG_API AObjectiveRestoreServers : public AObjectiveBase
{
	GENERATED_BODY()

public:
	AObjectiveRestoreServers();
	
protected:
	virtual void BeginPlay() override;
	virtual void ResetObjective() override;
	virtual void CompleteObjective() override;
	virtual void IncreaseObjectiveProgress(float const DeltaTime) override;

private:
	void SelectServersToRestore();
	void PrepareServersToRestore();
	void SetupTriggerEvents();
	void FindAllServers();

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
	UFUNCTION()
	void RegisterInteraction(AInteractableObject* InteractableObject);
	
	UPROPERTY()
	TArray<AObjectiveServer*> AllServers;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> ServersToRestore;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> RestoredServers;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AObjectiveServer> ObjectiveServerClass;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxTrigger;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	int NumberOfServers;

	UPROPERTY(EditAnywhere, Category = "Objective")
	int NumberOfServersToRestore;
};
