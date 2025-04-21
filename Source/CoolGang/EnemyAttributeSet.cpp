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
		if (CurrentHealth <= 0.0f)
		{
			AActor* OwnerActor = GetOwningActor();
			
			if (AEnemyAI* enemy = Cast<AEnemyAI>(OwnerActor))
			{
				// Notify actor to die
				
				
					enemy->Die(); // Your custom death function
				
				
			}
		}
	}
}