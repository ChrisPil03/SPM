// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyDamageCalculation.h"
#include "WeaponAttributeSet.h"
#include "EnemyAttributeSet.h"

// Struct to help capture attributes
struct FDamageStatics
{
    DECLARE_ATTRIBUTE_CAPTUREDEF(Health)
    DECLARE_ATTRIBUTE_CAPTUREDEF(Damage)

    FDamageStatics()
    {
        // Capture source weapon's damage
        DEFINE_ATTRIBUTE_CAPTUREDEF(UWeaponAttributeSet, Damage, Source, true);
        
        // Capture target's health
        DEFINE_ATTRIBUTE_CAPTUREDEF(UEnemyAttributeSet, Health, Target, false);
    }
};

static FDamageStatics& DamageStatics()
{
    static FDamageStatics Statics;
    return Statics;
}

UEnemyDamageCalculation::UEnemyDamageCalculation()
{
    // Register attributes we want to capture
    RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().HealthDef);
}

void UEnemyDamageCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);
    
    UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
    UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();

    if (!TargetASC || !SourceASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid ASC in Damage Calculation"));
        return;
    }

    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluateParameters;
    EvaluateParameters.SourceTags = SourceTags;
    EvaluateParameters.TargetTags = TargetTags;

    // Get damage from weapon
    float WeaponDamage = 0.0f;
    
    // First try to get damage from captured attribute
    if (!ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluateParameters, WeaponDamage))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to capture weapon Damage attribute in %s"), *SourceASC->GetOwner()->GetActorNameOrLabel());
        
        // Fallback: Try to get damage from SetByCaller in the GameplayEffect
      
        
        if (WeaponDamage <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("No damage value available. Using default."));
            WeaponDamage = 10.0f; // Default fallback damage
        }
    }
    
    // You can add modifiers here (critical hits, buffs, etc.)
    float FinalDamage = WeaponDamage;
    
    // Apply the damage as negative health
    if (FinalDamage > 0.0f)
    {
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().HealthProperty, 
                                                                         EGameplayModOp::Additive, 
                                                                         -FinalDamage));
    }
}