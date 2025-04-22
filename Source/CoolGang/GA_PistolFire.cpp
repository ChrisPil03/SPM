// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_PistolFire.h"

UGA_PistolFire::UGA_PistolFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_PistolFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) return;

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	
	if (Avatar == nullptr)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Shoot") );
	
	
	

	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_PistolFire::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	//const UGunAttributeSet* Attributes = ASC->GetSet<UGunAttributeSet>();

	//float CurrentAmmo = Attributes->GetAmmoCount();

	// Check that at least 1 bullet is available
	return true;
}
