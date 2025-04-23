#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "ObjectiveRestoreServers.generated.h"

class UBoxComponent;
class AInteractableObject;
class AObjectiveServer;

UENUM(BlueprintType)
enum class EServerHallStatus : uint8
{
	Idle,
	Operating,
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
	bool GetIsIdle() const { return ServerHallStatus == EServerHallStatus::Idle; }
	bool GetIsOperating() const { return ServerHallStatus == EServerHallStatus::Operating; }
	bool GetIsOverheated() const { return ServerHallStatus == EServerHallStatus::Overheated; }

	float GetHeatProgress() const { return CurrentHeatBuildup / 100; }
	
private:
	void InitializeServerHall();
	void SelectServersToRestore();
	void PrepareServersToRestore();
	void SetupTriggerEvents();
	void FindAllServers();
	void TriggerOverheat();
	void InitiateCoolingCycle();
	void BindControlPanel();
	void ResetHeatBuildup();

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
	void RegisterServerRestored(AInteractableObject* InteractableObject);

	UFUNCTION()
	void RegisterControlPanelInteraction(AInteractableObject* InteractableObject);

	UFUNCTION()
	void AddHeatBuildup(float Heat);
	
	UPROPERTY()
	TArray<AObjectiveServer*> AllServers;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> ServersToRestore;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TArray<AObjectiveServer*> RestoredServers;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxTrigger;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	int NumberOfServers;

	UPROPERTY(EditAnywhere, Category = "Objective")
	int NumberOfServersToRestore;

	UPROPERTY(VisibleAnywhere, Category = "Overheat")
	EServerHallStatus ServerHallStatus;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	AInteractableObject* ControlPanelCoolingSystem;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	float MaxHeatBuildup;
	
	UPROPERTY(VisibleAnywhere, Category = "Overheat")
	float CurrentHeatBuildup;

	UPROPERTY(EditAnywhere, Category = "Overheat")
	float DefaultHeatGeneration;
};
