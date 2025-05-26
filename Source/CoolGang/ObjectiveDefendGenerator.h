#pragma once

#include "CoreMinimal.h"
#include "Attackable.h"
#include "HealthComponent.h"
#include "ObjectiveBase.h"
#include "ObjectiveDefendGenerator.generated.h"

class UCapsuleComponent;
class AInteractableObject;
class AObjectiveDefendGenerator;
class UHealthComponent;

struct FOnAttributeChangeData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratorHealthChangedDelegate, float, Health);

UCLASS()

class COOLGANG_API AObjectiveDefendGenerator : public AObjectiveBase, public IAttackable
{
	GENERATED_BODY()
	
public:	
	AObjectiveDefendGenerator();

protected:
	virtual void BeginPlay() override;
	virtual void StartObjective() override;
	virtual void CompleteObjective() override;

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void ResetObjective() override;
	virtual void SetIsActive(const bool bNewState) override;
	virtual FVector GetWaypointTargetLocation() const override;
	virtual void FailObjective() override;
	virtual void DamageGeneratorShield(const float Damage) override;
	virtual TArray<FString> GetUniqueObjectiveProgress() const override;

	UFUNCTION(BlueprintPure)
	void GetTimeUntilShieldRestored(int32& OutMinutes, int32& OutSeconds) const;

	UFUNCTION(BlueprintPure)
	int32 GetSecondsUntilShieldDestroyed() const;
	
	UFUNCTION(BlueprintCallable)
	float GetHealthPercentage() const;

	UFUNCTION(BlueprintPure)
	float GetShieldPercentage() const;

	UFUNCTION(BlueprintPure)
	bool GetIsActivating() const { return bIsActivating; }
	
	UPROPERTY(EditAnywhere, Category = "Objective")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Objective")
	float MaxHealth;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnObjectiveEvent OnShieldChanged;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnObjectiveEvent OnShieldDestroyed;

private:
	void BindControlPanel();
	void BindCompletionFunction();
	void RegisterControlPanelInteraction(AInteractableObject* InteractableObject);
	bool CannotTakeDamage() const;
	void ActivateObjective();

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CapsuleComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float ActivationDelay;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* DownToHalfHealthVoiceLine;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* LowHealthVoiceLine;

	bool bHalfHealthVoiceLinePlayed;
	bool bLowHealthVoiceLinePlayed;

	FTimerHandle ActivationDelayTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	class UAbilitySystemComponent *AbilitySystemComponent;
	
	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_InitGeneratorStats;
	void InitStats();

	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_ResetGeneratorHealth;

	void OnCurrentHealthChanged(const FOnAttributeChangeData& Data);

	UPROPERTY(BlueprintAssignable, Category = "Gameplay")
	FOnGeneratorHealthChangedDelegate OnGeneratorHealthChangedDelegate;
	
	UPROPERTY(EditInstanceOnly, meta = (AllowPrivateAccess))
	AInteractableObject* ControlPanel;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	bool bIsActivating;

	// --------- Shield ------- //
	UPROPERTY(EditAnywhere, Category = "Objective")
	float CurrentShield;

	UPROPERTY(EditAnywhere, Category = "Objective")
	float MaxShield;
};
