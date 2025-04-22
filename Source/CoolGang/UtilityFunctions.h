// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class COOLGANG_API UtilityFunctions
{
public:
	UtilityFunctions();
	~UtilityFunctions();
	UFUNCTION(Category = "Utility", BlueprintCallable)
	void PrintActiveGameplayEffects(class UAbilitySystemComponent* AbilitySystemComponent);
};
