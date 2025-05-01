// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAttributeSet.h"
#include "EnemyAI.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectExtension.h"
#include "ObjectiveBase.h"

void UEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float CurrentHealth = GetHealth();
		float Magnitude = Data.EvaluatedData.Magnitude;
		// Access the effect context to get tags
		//const FGameplayEffectContextHandle EffectContext = Data.EffectSpec.GetEffectContext();
		//const FGameplayEffectContext* Context = EffectContext.Get();
		//FGameplayTagsContainer Tags;
		//Context->GetOwnedGameplayTags(Tags, true);

		// Check if the effect has a damage tag
		// if (Tags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Effect.Damage"))))
		// {
		// 	// This is a damage effect
		// 	UE_LOG(LogTemp, Warning, TEXT("Damage Magnitude: %f"), Magnitude);
		// }
		// else
		// {
		// 	// Other effect types
		// 	UE_LOG(LogTemp, Warning, TEXT("Other Magnitude Type: %f"), Magnitude);
		// }
		const UGameplayEffect* AppliedEffect = Data.EffectSpec.Def;
		
		if (AppliedEffect->GetName() == TEXT("Default__GE_Damage_C"))
		{
			if (Health.GetCurrentValue() <= MaxHealth.GetBaseValue() / 2)
			{
				UE_LOG(LogTemp, Error, TEXT("Enemy health being subtract by %f"), Magnitude);
				//float NewHealth = FMath::Clamp(CurrentHealth - Magnitude, 0.0f, GetMaxHealth());
				//SetHealth(NewHealth);
				//UE_LOG(LogTemp, Error, TEXT("Enemy health After being subtracted: %f"), GetHealth());
				
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