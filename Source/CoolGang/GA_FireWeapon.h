// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_FireWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UGA_FireWeapon : public UGameplayAbility
{
	GENERATED_BODY()

	public:
	UGA_FireWeapon();

	protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo,
							   const FGameplayEventData* TriggerEventData) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle,
					   const FGameplayAbilityActorInfo* ActorInfo,
					   FGameplayTagContainer* OptionalRelevantTags) const override;
	
	
	UFUNCTION(BlueprintCallable, Category = "Weapon|Trace")
	bool SingleTrace(FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Trace")
	bool MultiTrace(TArray<FHitResult>& Hits);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION(BlueprintCallable)
	void Fire();
	UFUNCTION(BlueprintCallable)
	void SingleBulletFire();
	UFUNCTION(BlueprintCallable)
	void PelletsFire();
private:
	bool GetTraceStartLocationAndRotation(FVector& OutStartPoint, FRotator& OutRotation) const;
	//for debug
	void BlinkDebug(FHitResult& h);
	FTimerHandle BlinkTimerHandle;
};


