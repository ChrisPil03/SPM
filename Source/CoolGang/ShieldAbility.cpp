#include "C:\Users\elst4488\Documents\GitHub\SPM\Intermediate\Build\Win64\x64\CoolGangEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.Cpp20.h"
#include "ShieldAbility.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UShieldAbility::UShieldAbility()
{
	// Set default properties for the ability (e.g., cooldown, cost, etc.)
}

void UShieldAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_LOG(LogTemp, Warning, TEXT("ShieldAbility: ActivateAbility called"));


	// Add the "ShieldActive" tag to the player
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayTag ShieldTag = FGameplayTag::RequestGameplayTag(TEXT("State.ShieldActive"));
		ActorInfo->AbilitySystemComponent->AddLooseGameplayTag(ShieldTag);
	}
	// Additional logic for when the shield starts (e.g., visuals, effects)
}

void UShieldAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("ShieldAbility: EndAbility called"));

	// Remove the "ShieldActive" tag from the player
    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayTag ShieldTag = FGameplayTag::RequestGameplayTag(TEXT("State.ShieldActive"));
		ActorInfo->AbilitySystemComponent->RemoveLooseGameplayTag(ShieldTag);
	}

	// Additional logic for when the shield ends (e.g., cleanup, effects)
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}