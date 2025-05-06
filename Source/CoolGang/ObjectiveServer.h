// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FProgressTimer.h"
#include "InteractableObject.h"
#include "ObjectiveServer.generated.h"

class UNiagaraComponent;

DECLARE_DELEGATE_OneParam(FServerHeatUpDelegate, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCompleteDelegate, AInteractableObject*, InteractableObject);

UENUM(BlueprintType)
enum class EServerState : uint8
{
	NeedRestoring,
	Restoring,
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
	
	UFUNCTION(BlueprintCallable, Category = "Server States")
	bool GetNeedsRestoring() const { return ServerState == EServerState::NeedRestoring; }
	UFUNCTION(BlueprintCallable, Category = "Server States")
	bool GetIsRestoring() const { return ServerState == EServerState::Restoring; }
	UFUNCTION(BlueprintCallable, Category = "Server States")
	bool GetIsRestored() const { return ServerState == EServerState::Restored; }
	UFUNCTION(BlueprintCallable, Category = "Server States")
	bool GetIsPaused() const { return ServerState == EServerState::Paused; }
	UFUNCTION(BlueprintCallable, Category = "Server States")
	bool GetIsIdle() const { return ServerState == EServerState::Idle; }

	UFUNCTION(BlueprintCallable, Category = "Progress")
	float GetProgress() const { return ProgressTimer->GetProgress(); }
	
	UFUNCTION()
	void ResumeRestoration();
	void PauseRestoration();
	void ResetServer();
	void SetSmokeEffectActive(const bool bNewState) const;

	void SetHeatUpFunction(const FServerHeatUpDelegate& NewDelegate) { HeatUpDelegate = NewDelegate; }
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCompleteDelegate CompleteDelegate;

private:
	void SetDebugMaterial() const;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* RestoringMaterial;
	void ResetMaterial();
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* StandardMaterial;
	
	void StartRestoration();
	void IncreaseRestorationProgress(float DeltaTime);
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

	bool bInstantRestoration;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	float HeatGeneration;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	UNiagaraComponent* SmokeNiagaraComponent;

	FServerHeatUpDelegate HeatUpDelegate;
};