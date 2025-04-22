// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PistolFire.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UGA_PistolFire : public UGameplayAbility
{
	GENERATED_BODY()

	public:
	UGA_PistolFire();

	protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo,
							   const FGameplayEventData* TriggerEventData) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle,
					   const FGameplayAbilityActorInfo* ActorInfo,
					   FGameplayTagContainer* OptionalRelevantTags) const override;
};

