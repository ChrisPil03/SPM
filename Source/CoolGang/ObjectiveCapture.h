// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "ObjectiveBase.h"
#include "ObjectiveCapture.generated.h"

class AInteractableObject;
class APlayerCharacter;
class USphereComponent;

UCLASS()
class COOLGANG_API AObjectiveCapture : public AObjectiveBase
{
	GENERATED_BODY()

public:
	AObjectiveCapture();

protected:
	virtual void BeginPlay() override;
	virtual void StartObjective() override;
	virtual void CompleteObjective() override;
	virtual void ResetObjective() override;
	virtual void IncreaseObjectiveProgress(float const DeltaTime) override;
	virtual void DecreaseObjectiveProgress(float const DeltaTime) override;
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetIsActive(const bool bNewState) override;
	virtual void FailObjective() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SpawnCaptureZone();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateCaptureZoneSize();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DestroyCaptureZone();

	UFUNCTION(BlueprintCallable)
	bool GetIsPlayerInZone() const { return PlayerInZone != nullptr; }
	
private:
	void FindInteractable();
	void BindInteractable();
	void RegisterControlPanelInteraction(AInteractableObject* Interactable);
	
	UFUNCTION()
	void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	float CaptureRadius;

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	USphereComponent* SphereTrigger;

	UPROPERTY(EditAnywhere)
	FName PlayerTag;

	UPROPERTY(VisibleAnywhere)
	APlayerCharacter* PlayerInZone;

	UPROPERTY(VisibleAnywhere)
	bool bCanInteractWith;

	UPROPERTY(EditAnywhere, Category = "CaptureZone")
	int32 DestroyZoneDelay;

	UPROPERTY(EditAnywhere, Category = "Objective")
	float FailObjectiveDelay;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* ExitZoneVoiceLine;
	
	FTimerHandle DelayTimerHandle;

	TUniquePtr<FProgressTimer> FailDelayProgressTimer;

	UPROPERTY(EditAnywhere, Category = "Interact")
	AInteractableObject* ControlPanel;
};
