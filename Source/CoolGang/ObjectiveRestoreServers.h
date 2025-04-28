#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "ObjectiveRestoreServers.generated.h"

class APlayerLocationDetection;
class UBoxComponent;
class AInteractableObject;
class AObjectiveServer;

UENUM(BlueprintType)
enum class EServerHallStatus : uint8
{
	Operating,
	Cooling,
	Overheated
};

UCLASS()
class COOLGANG_API AObjectiveRestoreServers : public AObjectiveBase
{
	GENERATED_BODY()

public:
	AObjectiveRestoreServers();
	
protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void ResetObjective() override;
	virtual void CompleteObjective() override;
	virtual void IncreaseObjectiveProgress(float const DeltaTime) override;

public:
	void SetServerHallStatus(const EServerHallStatus NewStatus) { ServerHallStatus = NewStatus; }

	UFUNCTION(BlueprintCallable, Category = "Server Hall States")
	bool GetIsOperating() const { return ServerHallStatus == EServerHallStatus::Operating; }
	UFUNCTION(BlueprintCallable, Category = "Server Hall States")
	bool GetIsCooling() const { return ServerHallStatus == EServerHallStatus::Cooling; }
	UFUNCTION(BlueprintCallable, Category = "Server Hall States")
	bool GetIsOverheated() const { return ServerHallStatus == EServerHallStatus::Overheated; }

	UFUNCTION(BlueprintCallable, Category = "Progress")
	float GetHeatProgress() const { return CurrentHeatBuildup / 100; }
	UFUNCTION(BlueprintCallable, Category = "Progress")
	float GetCoolingProgress() const { return CoolingProgress; }
	
private:
	void InitializeServerHall();
	void SelectServersToRestore();
	void PrepareServersToRestore();
	//void SetupTriggerEvents();
	void FindAllServers();
	void BindControlPanel();
	void BindPlayerLocationDetection();
	void ActivateControlPanel(const bool NewState);
	bool ValidServerToRestore(const AObjectiveServer* Server) const;
	bool GetIsServersRestored() const { return RestoredServers == NumberOfServersToRestore; }

	void OnEnterRoom(APlayerLocationDetection* Room);
	void OnExitRoom(APlayerLocationDetection* Room);

	void InitializeTimer();
	void ResetCoolingTimerProgress() const;
	void ResetHeatBuildup();
	void TriggerOverheat();
	void InitiateCoolingCycle();
	void CoolDown(float DeltaTime);
	void ResumeOperating();

	// UFUNCTION()
	// void OnBoxBeginOverlap(
	// 	UPrimitiveComponent* OverlappedComponent,
	// 	AActor* OtherActor,
	// 	UPrimitiveComponent* OtherComp,
	// 	int32 OtherBodyIndex,
	// 	bool bFromSweep,
	// 	const FHitResult& SweepResult);
	// UFUNCTION()
	// void OnBoxEndOverlap(
	// 	UPrimitiveComponent* OverlappedComponent,
	// 	AActor* OtherActor,
	// 	UPrimitiveComponent* OtherComp,
	// 	int32 OtherBodyIndex);
	
	UFUNCTION()
	void RegisterServerRestored(AInteractableObject* InteractableObject);

	UFUNCTION()
	void RegisterControlPanelInteraction(AInteractableObject* InteractableObject);

	UFUNCTION()
	void AddHeatBuildup(float Heat);

	UPROPERTY(EditInstanceOnly)
	APlayerLocationDetection* PlayerLocationDetection;
	
	UPROPERTY()
	TArray<AObjectiveServer*> AllServers;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> ServersToRestore;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	int32 RestoredServers;

	// UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	// UBoxComponent* BoxTrigger;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	int NumberOfServers;

	UPROPERTY(EditAnywhere, Category = "Objective")
	int NumberOfServersToRestore;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	bool bCanOverheat;

	UPROPERTY(VisibleAnywhere, Category = "Overheat")
	EServerHallStatus ServerHallStatus;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	AInteractableObject* ControlPanel;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	float MaxHeatBuildup;
	
	UPROPERTY(VisibleAnywhere, Category = "Overheat")
	float CurrentHeatBuildup;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	float DefaultHeatGeneration;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	float CoolingTime;

	UPROPERTY(VisibleAnywhere, Category = "Overheat")
	float CoolingProgress;

	TUniquePtr<FProgressTimer> CoolingTimer;
};
