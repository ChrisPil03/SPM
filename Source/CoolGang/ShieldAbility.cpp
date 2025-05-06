//#include "C:\Users\elst4488\Documents\GitHub\SPM\Intermediate\Build\Win64\x64\CoolGangEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.Cpp20.h"
#include "ShieldAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"

UShieldAbility::UShieldAbility()
{

}

bool UShieldAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayTagContainer* SourceTags,
                                        const FGameplayTagContainer* TargetTags,
                                        FGameplayTagContainer* OptionalRelevantTags) const
{
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		// Check if the ability is on cooldown
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(
			FGameplayTag::RequestGameplayTag(TEXT("Cooldown.ShieldAbility"))))
		{
			return false; // Ability is on cooldown
		}
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UShieldAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Apply the cooldown effect
	if (CooldownGameplayEffectClass && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayEffectSpecHandle CooldownSpec = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
			CooldownGameplayEffectClass, 1.f, ActorInfo->AbilitySystemComponent->MakeEffectContext());
		if (CooldownSpec.IsValid())
		{
			// Dynamically add the "Cooldown.ShieldAbility" tag
			CooldownSpec.Data->DynamicGrantedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Cooldown.ShieldAbility")));
			ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CooldownSpec.Data.Get());
		}
	}

	// Add the "ShieldActive" tag
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayTag ShieldTag = FGameplayTag::RequestGameplayTag(TEXT("State.ShieldActive"));
		ActorInfo->AbilitySystemComponent->AddLooseGameplayTag(ShieldTag);
	}
}

void UShieldAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                bool bWasCancelled)
{

	// Remove the "ShieldActive" tag
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayTag ShieldTag = FGameplayTag::RequestGameplayTag(TEXT("State.ShieldActive"));
		ActorInfo->AbilitySystemComponent->RemoveLooseGameplayTag(ShieldTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
void UShieldAbility::DeactivateShield()
{
	UE_LOG(LogTemp, Warning, TEXT("MyBlueprintFunction called!"));

	
}