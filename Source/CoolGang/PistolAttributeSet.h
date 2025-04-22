// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PistolAttributeSet.generated.h"

/**
 * 
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#define DECLARE_GAS_ATTRIBUTE(Type, Name) \
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes") \
FGameplayAttributeData Name; \
ATTRIBUTE_ACCESSORS(Type, Name)

UCLASS()
class COOLGANG_API UPistolAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	DECLARE_GAS_ATTRIBUTE(UPistolAttributeSet, Damage)
	DECLARE_GAS_ATTRIBUTE(UPistolAttributeSet, FireRate)
	DECLARE_GAS_ATTRIBUTE(UPistolAttributeSet, MagazineSize)
	DECLARE_GAS_ATTRIBUTE(UPistolAttributeSet, AmmoInMag)

	

};
