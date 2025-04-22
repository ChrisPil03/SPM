// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_PistolFire.h"
#include "AbilitySystemComponent.h"

#include "PlayerCharacter.h"

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

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Avatar);
	
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	PlayerCharacter->PullTrigger();

	
	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}
	
	AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(GE_AmmoConsumed), 1, AbilitySystemComponent->MakeEffectContext());
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_PistolFire::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	
	

	//float CurrentAmmo = Attributes->GetAmmoCount();

	// Check that at least 1 bullet is available
	return true;
}

