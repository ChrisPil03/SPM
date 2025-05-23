#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ShieldAbility.generated.h"

UCLASS()
class COOLGANG_API UShieldAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UShieldAbility();
	UFUNCTION(BlueprintCallable)
	void DeactivateShield();

	
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                                const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags,
	                                const FGameplayTagContainer* TargetTags,
	                                FGameplayTagContainer* OptionalRelevantTags) const override;


	// Called when the ability is activated
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	// Called when the ability ends
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
};
