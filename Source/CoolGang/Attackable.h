// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "UObject/Interface.h"
#include "Attackable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UAttackable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IAttackable
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual TArray<USphereComponent*> GetMovementNodes() = 0;
};
