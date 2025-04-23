// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FProgressTimer.h"
#include "InteractableObject.h"
#include "ObjectiveServer.generated.h"

DECLARE_DELEGATE_OneParam(FServerHeatUpDelegate, float)

UENUM(BlueprintType)
enum class EServerState : uint8
{
	NeedRestoring,
	Restoring,
	Cooling,
	Restored,
	Paused,
	Idle
};

UCLASS()
class COOLGANG_API AObjectiveServer : public AInteractableObject
{
	GENERATED_BODY()

public:
	AObjectiveServer();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Interact(AActor* Interactor) override;

	void SetServerState(const EServerState NewState);
	bool GetNeedsRestoring() const { return ServerState == EServerState::NeedRestoring; }
	bool GetIsRestoring() const { return ServerState == EServerState::Restoring; }
	bool GetIsCooling() const { return ServerState == EServerState::Cooling; }
	bool GetIsRestored() const { return ServerState == EServerState::Restored; }
	bool GetIsPaused() const { return ServerState == EServerState::Paused; }
	bool GetIsIdle() const { return ServerState == EServerState::Idle; }
	
	float GetProgress() const { return ProgressTimer->GetProgress(); }
	float GetCoolingProgress() const { return CoolingTimer->GetProgress(); }

	void BeginCooling();
	void PauseRestoration();

	void SetHeatUpFunction(const FServerHeatUpDelegate& NewDelegate) { HeatUpDelegate = NewDelegate; }
	
	UFUNCTION()
	void ResumeRestoration();

private:
	//Currently for debugging
	void SetDebugMaterial() const;
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* RedMaterial;

	void InitiateTimers();
	void StartRestoration();
	void IncreaseRestorationProgress(float DeltaTime);
	void CoolDown(float DeltaTime);
	void GenerateHeat(float DeltaTime);

	UFUNCTION()
	void CompleteRestoration();
	
	UPROPERTY(EditAnywhere, Category = "Server")
	float RestoreTime;

	UPROPERTY(VisibleAnywhere, Category = "Server")
	float RestoreProgress;

	UPROPERTY(VisibleAnywhere, Category = "Server")
	EServerState ServerState;
	
	TUniquePtr<FProgressTimer> ProgressTimer;
	TUniquePtr<FProgressTimer> CoolingTimer;

	bool bInstantRestoration;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	float HeatGeneration;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	float CoolingTime;

	FServerHeatUpDelegate HeatUpDelegate;
};
