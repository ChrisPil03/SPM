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

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle,
					   const FGameplayAbilityActorInfo* ActorInfo,
					   const FGameplayAbilityActivationInfo ActivationInfo) const override;
	

	UFUNCTION(BlueprintCallable, Category = "Weapon|Trace")
	bool BulletTrace(TArray<FHitResult>& Hits);

	
	
	bool NormalBulletTrace(FHitResult& HitResult, 
						 const FVector& StartPoint, 
						 const FVector& EndPoint,
						 const FCollisionQueryParams& QueryParams);
                          
	bool PiercingBulletTrace(TArray<FHitResult>& HitResults,
							const FVector& StartPoint,
							const FVector& EndPoint,
							const FCollisionQueryParams& QueryParams);

	bool LaserBulletTrace(TArray<FHitResult>& HitResults,
							const FVector& StartPoint,
							const FVector& EndPoint,
							const FCollisionQueryParams& QueryParams);

	bool ChainingBulletTrace(TArray<FHitResult>& HitResults,
							const FVector& StartPoint,
							const FVector& EndPoint,
							const FCollisionQueryParams& QueryParams);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION(BlueprintCallable)
	bool Fire();

	UFUNCTION(BlueprintNativeEvent)
	void SpawnImpactEffect(const FHitResult& HitResult) const;

private:
	bool GetTraceStartLocationAndRotation(FVector& OutStartPoint, FRotator& OutRotation) const;
	bool IsDuplicateHit(const TArray<FHitResult>& ExistingHits, const AActor* Actor);

	void DrawImpactPointDeBug(const FVector& Location) const;

	

	UPROPERTY()
	FGameplayAbilityTargetDataHandle TargetDator;
	
	FGameplayAbilityTargetData_SingleTargetHit* NewTargetData;
	
	void ProcessHitChain(const FHitResult& InitialHit, TArray<FHitResult>& ChainHits, 
		int32 MaxChainDepth, float SphereRadius, FCollisionQueryParams& OverlapQueryParams);

};


