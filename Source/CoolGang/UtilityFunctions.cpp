// Fill out your copyright notice in the Description page of Project Settings.


#include "UtilityFunctions.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void UtilityFunctions::PrintActiveGameplayEffects(UAbilitySystemComponent* AbilitySystemComponent)
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    FGameplayEffectQuery Query; // Create a default query object
    for (const FActiveGameplayEffectHandle& ActiveEffectHandle : AbilitySystemComponent->GetActiveEffects(Query))
    {
        const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffectHandle);
        if (ActiveEffect && ActiveEffect->Spec.Def)
        {
            const UGameplayEffect* GameplayEffect = ActiveEffect->Spec.Def;
            // UE_LOG(LogTemp, Log, TEXT("Active Gameplay Effect: %s"), *GameplayEffect->GetName());
        }
    }
}

UtilityFunctions::UtilityFunctions()
{
}

UtilityFunctions::~UtilityFunctions()
{
}
