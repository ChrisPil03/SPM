// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAttributeSet.h"
#include "GameplayEffectExtension.h"

void UWeaponAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Check if the modified attribute is Ammo
	if (Data.EvaluatedData.Attribute == GetAmmoCountAttribute())
	{
		// Get the new ammo value after the effect has been applied
		float NewAmmo = GetAmmoCount();
		
		// Update the UI (call a function that will notify the player controller or HUD)
		OnAmmoChanged.Broadcast(NewAmmo);
	}
}