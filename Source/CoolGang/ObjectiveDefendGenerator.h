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
	// virtual float TakeDamage(
	// 	float DamageAmount,
	// 	FDamageEvent const& DamageEvent,
	// 	AController* EventInstigator,
	// 	AActor* DamageCauser) override;
	virtual void SetIsActive(const bool bNewState) override;
	virtual FVector GetWaypointTargetLocation() const override;
	
	UFUNCTION(BlueprintCallable)
	float GetHealthPercentage() const;

	UPROPERTY(EditAnywhere)
	float Health;

	UPROPERTY(EditAnywhere)
	float MaxHealth;

private:
	void BindControlPanel();
	void BindDeathFunction();
	void BindCompletionFunction();
	void RegisterControlPanelInteraction(AInteractableObject* InteractableObject);
	bool CannotTakeDamage() const;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CapsuleComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly)
	float StartDelay;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* DownToHalfHealthVoiceLine;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* LowHealthVoiceLine;

	bool bHalfHealthVoiceLinePlayed;
	bool bLowHealthVoiceLinePlayed;

	FTimerHandle StartWaitTimerHandle;

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
};
