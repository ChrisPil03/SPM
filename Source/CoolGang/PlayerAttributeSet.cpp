// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectExtension.h"
#include "PlayerCharacter.h"

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float CurrentHealth = GetHealth();
		float Magnitude = Data.EvaluatedData.Magnitude;
		
		const UGameplayEffect* AppliedEffect = Data.EffectSpec.Def;

		UE_LOG(LogTemp, Error, TEXT("Player health being subtract by %f"), Magnitude);
		if (AppliedEffect->GetName() == TEXT("Default__GE_ApplyDamageToPlayer_C"))
		{
			if (CurrentHealth <= 0.0f)
			{
				AActor* OwnerActor = GetOwningActor();
			
				if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerActor))
				{
					Player->Die(); 
				}
			}
		}
	}
}
