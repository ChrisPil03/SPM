// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FProgressTimer.h"
#include "InteractableObject.h"
#include "ObjectiveServer.generated.h"

class UNiagaraComponent;

DECLARE_DELEGATE_OneParam(FServerHeatUpDelegate, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCompleteDelegate, AInteractableObject*, InteractableObject);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnServerEvent, AObjectiveServer*);

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
	int32 GetFailTime() const { return FailTime; }
	
	UFUNCTION()
	void ResumeRestoration();
	void PauseRestoration();
	void ResetServer();
	void SetSmokeEffectActive(const bool bNewState) const;
	void StartRestoration();
	void GetElapsedMinutesAndSeconds(int32& OutMinutes, int32& OutSeconds);

	// void SetHeatUpFunction(const FServerHeatUpDelegate& NewDelegate) { HeatUpDelegate = NewDelegate; }
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCompleteDelegate CompleteDelegate;

	template <typename T>
	void AddOnFailFunction(T* Object, void (T::*Func)(AObjectiveServer*))
	{
		if (!OnFailEvent.IsBoundToObject(Object))
		{
			OnFailEvent.AddUObject(Object, Func);	
		}
	}

	template <typename T>
	void AddOnPausedFunction(T* Object, void (T::*Func)(AObjectiveServer*))
	{
		if (!OnPausedEvent.IsBoundToObject(Object))
		{
			OnPausedEvent.AddUObject(Object, Func);	
		}
	}

	template <typename T>
	void AddOnResumedFunction(T* Object, void (T::*Func)(AObjectiveServer*))
	{
		if (!OnResumedEvent.IsBoundToObject(Object))
		{
			OnResumedEvent.AddUObject(Object, Func);	
		}
	}

	UFUNCTION(BlueprintImplementableEvent)
	void SetServerLightColor(const bool bMalfunctioning);

private:
	void IncreaseRestorationProgress(float DeltaTime);
	void IncreaseFailProgress(float DeltaTime);
	void BroadcastServerFailed();
	void BroadcastServerPaused();
	void BroadcastServerResumed();
	void SetPausedMaterials();
	void ResetMaterial();
	// void GenerateHeat(float DeltaTime);
	
	UFUNCTION()
	void CompleteRestoration();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* CablePausedMaterial;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* ServerMaterial;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* StandardMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Server")
	int32 RestoreTime;

	UPROPERTY(VisibleAnywhere, Category = "Server")
	float RestoreProgress;

	UPROPERTY(VisibleAnywhere, Category = "Server")
	EServerState ServerState;

	UPROPERTY(EditAnywhere, Category = "Server")
	UNiagaraComponent* SmokeNiagaraComponent;
	
	TUniquePtr<FProgressTimer> ProgressTimer;

	FTimerHandle MalfunctionTimerHandle;
	
	FOnServerEvent OnFailEvent;
	FOnServerEvent OnPausedEvent;
	FOnServerEvent OnResumedEvent;

	TUniquePtr<FProgressTimer> FailProgressTimer;

	UPROPERTY(EditAnywhere, Category = "Server")
	int32 FailTime;
	
	// bool bInstantRestoration;
	//
	// UPROPERTY(EditAnywhere, Category = "Overheat")
	// float HeatGeneration;
	//
	//
	// FServerHeatUpDelegate HeatUpDelegate;
};