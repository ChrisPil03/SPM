// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAttributeSet.h"
#include "EnemyAI.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectExtension.h"
#include "ObjectiveBase.h"

void UEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
	    const UGameplayEffect* AppliedEffect = Data.EffectSpec.Def;
		
		if (AppliedEffect->GetName() == TEXT("Default__GE_DamageReduction_C"))

	    {
	        float DamageReduction = Data.EvaluatedData.Magnitude;
	        // Apply the damage reduction

			Damage = Damage.GetCurrentValue() * DamageReduction;
	        
	    }
	
	}
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float CurrentHealth = GetHealth();
		float Magnitude = Data.EvaluatedData.Magnitude;
		
		const UGameplayEffect* AppliedEffect = Data.EffectSpec.Def;
		
		if (AppliedEffect->GetName() == TEXT("Default__GE_Damage_C"))
		{
			if (Health.GetCurrentValue() <= MaxHealth.GetBaseValue() / 3)
			{
				UE_LOG(LogTemp, Error, TEXT("Enemy health being subtract by %f"), Magnitude);
				
				if (AEnemyAI* enemy = Cast<AEnemyAI>(GetOwningActor()))
				{
					TScriptInterface<IAttackable> TargetInterface = enemy->GetTarget();
					if (UObject* TargetObject = TargetInterface.GetObject())
					{
						if (Cast<AObjectiveBase>(TargetObject))
						{
							enemy->AttackPlayer(nullptr);
						}
					}
				}
			}
		}
		
		if (CurrentHealth <= 0.0f)
		{
			AActor* OwnerActor = GetOwningActor();
			
			if (AEnemyAI* enemy = Cast<AEnemyAI>(OwnerActor))
			{
				
					enemy->Die(); 
			}
		}
	}
}