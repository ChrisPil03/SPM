// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FProgressTimer.h"
#include "InteractableObject.h"
#include "ObjectiveServer.generated.h"

UENUM(BlueprintType)
enum class EServerState : uint8
{
	NeedRestoring,
	Restoring,
	Restored,
	Idle
};

UCLASS()
class COOLGANG_API AObjectiveServer : public AInteractableObject
{
	GENERATED_BODY()

public:
	AObjectiveServer();
	virtual ~AObjectiveServer() override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Interact(AActor* Interactor) override;

	void SetServerState(const EServerState NewState);
	bool GetNeedsRestoring() const { return ServerState == EServerState::NeedRestoring; }
	bool GetIsRestoring() const { return ServerState == EServerState::Restoring; }
	bool GetIsRestored() const { return ServerState == EServerState::Restored; }
	bool GetIsIdle() const { return ServerState == EServerState::Idle; }

private:
	//Currently for debugging
	void SetDebugMaterial() const;
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* RedMaterial;
	
	void StartRestoration();
	void IncreaseRestorationProgress(float DeltaTime);

	UFUNCTION()
	void CompleteRestoration();
	
	UPROPERTY(EditAnywhere, Category = "Server")
	float RestoreTime;

	UPROPERTY(VisibleAnywhere, Category = "Server")
	float RestoreProgress;

	UPROPERTY(VisibleAnywhere, Category = "Server")
	EServerState ServerState;
	
	FProgressTimer* Timer;

	bool bInstantRestoration;
};
