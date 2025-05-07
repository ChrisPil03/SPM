// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "EnemyDamageCalculation.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UEnemyDamageCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	public:
	UEnemyDamageCalculation();
	protected:
	virtual  void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
	
};
