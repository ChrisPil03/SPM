#include "GenericAbility.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"

UGenericAbility::UGenericAbility()
{
	// Set default values (e.g., Instancing policy)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}