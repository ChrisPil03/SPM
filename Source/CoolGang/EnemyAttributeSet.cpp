// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAttributeSet.h"
#include "EnemyAI.h"
#include "GameplayEffectExtension.h"

void UEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float CurrentHealth = GetHealth();
		float DamageAmount = Data.EvaluatedData.Magnitude;
		if (CurrentHealth > 0.0f)
		{
			float NewHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);
		}
		
		if (CurrentHealth <= 0.0f)
		{
			AActor* OwnerActor = GetOwningActor();
			
			if (AEnemyAI* enemy = Cast<AEnemyAI>(OwnerActor))
			{
				UE_LOG(LogTemp, Error, TEXT("Enemy health being subtract "));
					enemy->Die(); 
			}
		}
	}
}