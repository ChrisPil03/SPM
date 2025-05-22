// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAttributeSet.h"
#include "EnemyAI.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectExtension.h"
#include "ObjectiveBase.h"
#include "Engine/DamageEvents.h" 
#include "Kismet/GameplayStatics.h"

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
		if (AEnemyAI* Enemy = Cast<AEnemyAI>(GetOwningActor()))
		{
			float CurrentHealth = GetHealth();
			float Magnitude = Data.EvaluatedData.Magnitude;
			
			const UGameplayEffect* AppliedEffect = Data.EffectSpec.Def;
			
			TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();
			if (Magnitude <= 0)
			{
				FDamageEvent DmgEvent(DamageTypeClass);
				Enemy->TakeDamage(FMath::Abs(Magnitude), DmgEvent, UGameplayStatics::GetPlayerController(GetWorld(), 0), UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			
			}
			
			if (AppliedEffect->GetName() == TEXT("Default__GE_ApplyDamageToEnemy_C"))
			{
				if (Health.GetCurrentValue() <= MaxHealth.GetBaseValue() / 3)
				{
					UE_LOG(LogTemp, Error, TEXT("Enemy health being subtract by %f"), Magnitude);
					
					
					TScriptInterface<IAttackable> TargetInterface = Enemy->GetTarget();
					if (UObject* TargetObject = TargetInterface.GetObject())
					{
						if (Cast<AObjectiveBase>(TargetObject))
						{
							Enemy->AttackPlayer(nullptr);
						}
					}
					
				}
			}
		
			if (CurrentHealth <= 0.0f)
			{
				Enemy->StartDeathSequence(); 
			}
		}
	}
}
