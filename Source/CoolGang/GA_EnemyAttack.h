// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EnemyAttack.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UGA_EnemyAttack : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_EnemyAttack();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo,
							   const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);
	
	UFUNCTION(BlueprintCallable)
	void Attack();
};
