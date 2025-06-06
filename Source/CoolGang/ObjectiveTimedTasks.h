#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "ObjectiveTimedTasks.generated.h"

class AInteractableObject;
class AObjectiveButton;

UCLASS()
class COOLGANG_API AObjectiveTimedTasks : public AObjectiveBase
{
	GENERATED_BODY()

public:
	AObjectiveTimedTasks();
	
protected:
	virtual void BeginPlay() override;
	virtual void ResetObjective() override;
	virtual void CompleteObjective() override;

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetIsActive(const bool bNewState) override;
	virtual float GetObjectiveProgress() const override;
	virtual TArray<FString> GetUniqueObjectiveProgress() const override;

	UFUNCTION(BlueprintPure)
	void GetTimeUntilFailure(int32& OutMinutes, int32& OutSeconds) const;
	
	UFUNCTION(BlueprintPure)
	int32 GetInteractions() const { return InteractedTasks; }
	UFUNCTION(BlueprintPure)
	int32 GetTotalInteractables() const { return AllInteractableObjects.Num(); }
	
private:
	UFUNCTION()
	void RegisterInteraction(AInteractableObject* InteractableObject);
	void SetInteractablesInteractable(const bool bNewState);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"), Category="Objective")
	TArray<AInteractableObject*> AllInteractableObjects;

	UPROPERTY(VisibleAnywhere)
	int32 InteractedTasks;
};
