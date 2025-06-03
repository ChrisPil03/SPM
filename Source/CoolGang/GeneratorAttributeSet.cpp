// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneratorAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectExtension.h"
#include "ObjectiveDefendGenerator.h"

void UGeneratorAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	UE_LOG(LogTemp, Error, TEXT("PostGameplayEffectExecute"));
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float CurrentHealth = GetHealth();
		float Magnitude = Data.EvaluatedData.Magnitude;
		
		const UGameplayEffect* AppliedEffect = Data.EffectSpec.Def;

		UE_LOG(LogTemp, Error, TEXT("Generator health being subtract by %f"), Magnitude);
		
		if (CurrentHealth <= 0.0f)
		{
			AActor* OwnerActor = GetOwningActor();
		
			if (AObjectiveDefendGenerator* Generator = Cast<AObjectiveDefendGenerator>(OwnerActor))
			{
				Generator->FailObjective();
			}
		}
	}
}
