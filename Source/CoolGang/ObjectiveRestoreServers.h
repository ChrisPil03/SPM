#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "ObjectiveRestoreServers.generated.h"

class APlayerLocationDetection;
class UBoxComponent;
class AInteractableObject;
class AObjectiveServer;

// UENUM(BlueprintType)
// enum class EServerHallStatus : uint8
// {
// 	Operating,
// 	Cooling,
// 	Overheated
// };

USTRUCT(BlueprintType)
struct FIntRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 Max = 0;

	UPROPERTY(EditAnywhere)
	int32 Min = 0;

	int32 GetRandomValue() const { return FMath::RandRange(Min, Max); }
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
	virtual void StartObjective() override;
	virtual void ResetObjective() override;
	virtual void CompleteObjective() override;
	virtual void IncreaseObjectiveProgress(float const DeltaTime) override;

public:
	// void SetServerHallStatus(const EServerHallStatus NewStatus) { ServerHallStatus = NewStatus; }

	// UFUNCTION(BlueprintCallable, Category = "Server Hall States")
	// bool GetIsOperating() const { return ServerHallStatus == EServerHallStatus::Operating; }
	// UFUNCTION(BlueprintCallable, Category = "Server Hall States")
	// bool GetIsCooling() const { return ServerHallStatus == EServerHallStatus::Cooling; }
	// UFUNCTION(BlueprintCallable, Category = "Server Hall States")
	// bool GetIsOverheated() const { return ServerHallStatus == EServerHallStatus::Overheated; }
	//
	// UFUNCTION(BlueprintCallable, Category = "Progress")
	// float GetHeatProgress() const { return CurrentHeatBuildup / 100; }
	// UFUNCTION(BlueprintCallable, Category = "Progress")
	// float GetCoolingProgress() const { return CoolingProgress; }

	virtual void SetIsActive(const bool bNewState) override;
	virtual void FailObjective() override;
	virtual FVector GetWaypointTargetLocation() const override;
	virtual TArray<FString> GetUniqueObjectiveProgress() const override;

	UFUNCTION(BlueprintPure)
	void GetTimeUntilFailure(int32& OutMinutes, int32& OutSeconds) const;
	UFUNCTION(BlueprintPure)
	int32 GetServerInteractions() const { return ServerInteractions; }
	UFUNCTION(BlueprintPure)
	int32 GetNumberOfSelectedServers() const { return NumberOfServersToSelect; }
	
private:
	void InitializeServerHall();
	void SelectServers();
	void PrepareSelectedServers();
	void FindAllServers();
	void BindControlPanel();
	void ActivateControlPanel(const bool NewState);
	bool ValidServerToRestore(const AObjectiveServer* Server) const;
	bool GetIsServersRestored() const { return RestoredServers == NumberOfServersToSelect; }
	void ResetSelectedServers();
	void ResetServerRoom();
	void SetRandomServersToSelect();

	UFUNCTION()
	void OnInteract(AInteractableObject* InteractableObject);

	// void InitializeTimers();
	// void ResetHeatBuildup();
	// void TriggerOverheat();
	// void InitiateCoolingCycle();
	// void CoolDown(float DeltaTime);
	// void ResumeOperating();
	
	UFUNCTION()
	void RegisterServerRestored(AInteractableObject* InteractableObject);
	void RegisterServerInteraction(AInteractableObject* InteractableObject);

	UFUNCTION()
	void RegisterControlPanelInteraction(AInteractableObject* InteractableObject);

	UFUNCTION()
	void RegisterFailedServer(AObjectiveServer* Server);

	UFUNCTION()
	void RegisterServerPaused(AObjectiveServer* Server);

	UFUNCTION()
	void RegisterServerResumed(AObjectiveServer* Server);
	
	// UFUNCTION()
	// void AddHeatBuildup(float Heat);
	
	UPROPERTY()
	TArray<AObjectiveServer*> AllServers;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> SelectedServers;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	int32 RestoredServers;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	int32 ServerInteractions;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	int32 NumberOfServers;

	UPROPERTY(EditAnywhere, Category = "Objective")
	FIntRange ServersToSelectRange;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	int32 NumberOfServersToSelect;

	UPROPERTY(EditAnywhere, Category = "Objective")
	AInteractableObject* ControlPanel;

	UPROPERTY()
	TArray<AObjectiveServer*> FailingServers;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	bool bServerFailing;

	// UPROPERTY(EditAnywhere, Category = "Overheat")
	// bool bCanOverheat;
	//
	// UPROPERTY(VisibleAnywhere, Category = "Overheat")
	// EServerHallStatus ServerHallStatus;
	//
	// UPROPERTY(EditAnywhere, Category = "Overheat")
	// float MaxHeatBuildup;
	//
	// UPROPERTY(VisibleAnywhere, Category = "Overheat")
	// float CurrentHeatBuildup;
	//
	// UPROPERTY(EditAnywhere, Category = "Overheat")
	// float DefaultHeatGeneration;
	//
	// UPROPERTY(EditAnywhere, Category = "Overheat")
	// float CoolingTime;
	//
	// UPROPERTY(VisibleAnywhere, Category = "Overheat")
	// float CoolingProgress;
	//
	// UPROPERTY(EditAnywhere, Category = "Overheat")
	// float OverheatSystemIntegrityDamage;
	//
	// UPROPERTY(EditAnywhere, Category = "Overheat")
	// float FailDelay;

	// UPROPERTY(EditAnywhere, Category = "Message")
	// FString OverheatedMessage;
	//
	// UPROPERTY(EditAnywhere, Category = "Audio")
	// USoundBase* OverheatVoiceLine;
	//
	// TUniquePtr<FProgressTimer> CoolingTimer;
	// TUniquePtr<FProgressTimer> FailDelayProgressTimer;
};
