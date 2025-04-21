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

public:
	virtual void Tick(float DeltaSeconds) override;

private:
	void SetServersToRestore();

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
	
	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> AllServers;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> ServersToRestore;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> RestoredServers;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AObjectiveServer> ObjectiveServerClass;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxTrigger;

	UPROPERTY(EditAnywhere, Category = "Objective")
	int NumberOfServersToRestore;
};
