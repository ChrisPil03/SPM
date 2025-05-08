// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_EnemyAttack.h"
#include "EnemyAI.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"  

UGA_EnemyAttack::UGA_EnemyAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_EnemyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_EnemyAttack::Attack()
{
	AEnemyAI* Enemy = Cast<AEnemyAI>(GetOwningActorFromActorInfo());
	if (!Enemy) return;

	AActor* TargetActor = Enemy->GetCurrentTarget();
	if (!TargetActor) return;

	// Create a TargetDataHandle from the actor directly
	FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor);

	// Now broadcast or apply this data
	OnTargetDataReady(TargetData);
}


