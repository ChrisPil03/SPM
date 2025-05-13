// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_FireWeapon.h"
#include "AbilitySystemComponent.h"
#include "GunBase.h"
#include "WeaponAttributeSet.h"

#include "PlayerCharacter.h"
#define PIERCING_TRACE ECC_GameTraceChannel6
#define NORMAL_TRACE ECC_GameTraceChannel1
UGA_FireWeapon::UGA_FireWeapon()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_FireWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_FireWeapon::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const UWeaponAttributeSet* Attributes = ASC->GetSet<UWeaponAttributeSet>();
	float CurrentAmmo = Attributes->GetAmmoCount();
	// Check that at least 1 bullet is available
	return CurrentAmmo >= 1;
}

void UGA_FireWeapon::Fire()
{
	TArray<FHitResult> HitResults;
	BulletTrace(HitResults);
	
	FGameplayAbilityTargetDataHandle TargetData;

	for (const FHitResult& Hit : HitResults)
	{
		// Create a new target data for this individual hit
		FGameplayAbilityTargetData_SingleTargetHit* NewTargetData =
			new FGameplayAbilityTargetData_SingleTargetHit(Hit);
    
		// Add it to the handle (this builds an array internally)
		TargetData.Add(NewTargetData);
	}

	// Pass it to your ability logic
	OnTargetDataReady(TargetData);

}

bool UGA_FireWeapon::GetTraceStartLocationAndRotation(FVector& OutStartPoint, FRotator& OutRotation) const
{
	APawn* OwningPawn = Cast<APawn>(GetOwningActorFromActorInfo()->GetOwner()); 
	if (OwningPawn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("OwningPawn is nullptr") );
		return false;
	}
	AController* OwnerController = OwningPawn->GetController();
	if (OwnerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("OwnerController is nullptr") );
		return false;
	}
	OwnerController->GetPlayerViewPoint(OutStartPoint, OutRotation);
	return true;
}

bool UGA_FireWeapon::BulletTrace(TArray<FHitResult>& HitResults)
{
	bool bHasTarget = false;
	FVector StartPoint;
	FRotator Rotation;
	GetTraceStartLocationAndRotation(StartPoint, Rotation);
	const FVector BulletDirection = Rotation.Vector();
	const UAbilitySystemComponent* ASC = GetActorInfo().AbilitySystemComponent.Get();
	const UWeaponAttributeSet* Attributes = ASC->GetSet<UWeaponAttributeSet>();
	float NumPellets = Attributes->GetPellets();
	const float ConeHalfAngleDegrees = Attributes->GetBulletSpreadAngle();
	float MaxRange = Attributes->GetMaxRange();
	ECollisionChannel TraceChannel = NORMAL_TRACE;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwningActorFromActorInfo());
	QueryParams.AddIgnoredActor(GetOwningActorFromActorInfo()->GetOwner());
	TArray<FHitResult> PiercingHitResults;
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.PiercingActive")))
	{
		TraceChannel = PIERCING_TRACE;
		UE_LOG(LogTemp, Warning, TEXT("Weapon.PiercingActive"));
	}
	for (int32 i = 0; i < NumPellets; ++i)
	{
		FVector ShootDirection = FMath::VRandCone(BulletDirection, FMath::DegreesToRadians(ConeHalfAngleDegrees));
		FVector EndPoint = StartPoint + (ShootDirection * MaxRange); // Trace distance
		
		bool bHit = GetWorld()->LineTraceMultiByChannel(PiercingHitResults, StartPoint, EndPoint, TraceChannel, QueryParams);
		
		if (bHit)
		{
			for (const FHitResult& HitResult : PiercingHitResults)
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.Component->GetName());
				
				if (!IsDuplicateHit(HitResults, HitResult.GetActor()))
				{
					HitResults.Add(HitResult);
				}
			}
			
			DrawDebugLine(GetWorld(), StartPoint, EndPoint,FColor::Red, false, 2.0f);
			//BlinkDebug(Hit);
			bHasTarget = true;
		}
	}

	return bHasTarget;
}


bool UGA_FireWeapon::IsDuplicateHit(const TArray<FHitResult>& ExistingHits, const AActor* Actor)
{
	for (const FHitResult& Hit : ExistingHits)
	{
		if (Hit.GetActor() == Actor)
		{
			return true;
		}
	}
	return false;
}