// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_FireWeapon.h"
#include "AbilitySystemComponent.h"
#include "GunBase.h"
#include "WeaponAttributeSet.h"

#include "PlayerCharacter.h"
#include "Engine/OverlapResult.h"
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
	// Check that at least 1 bullet is available
	int32 CurrentAmmo = FMath::TruncToInt(Attributes->GetAmmoCount());
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
	bool bHasTarget;
	FVector StartPoint;
	FRotator Rotation;
	GetTraceStartLocationAndRotation(StartPoint, Rotation);
	const FVector BulletDirection = Rotation.Vector();
	const UAbilitySystemComponent* ASC = GetActorInfo().AbilitySystemComponent.Get();
	const UWeaponAttributeSet* Attributes = ASC->GetSet<UWeaponAttributeSet>();
	float NumPellets = Attributes->GetPellets();
	const float ConeHalfAngleDegrees = Attributes->GetBulletSpreadAngle();
	float MaxRange = Attributes->GetMaxRange();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwningActorFromActorInfo());
	QueryParams.AddIgnoredActor(GetOwningActorFromActorInfo()->GetOwner());
	

    
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.PiercingActive")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon.PiercingActive"));
		bHasTarget = PiercingBulletTrace(HitResults, StartPoint, BulletDirection, NumPellets, 
										 ConeHalfAngleDegrees, MaxRange, QueryParams);
	}
	else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.ChainingActive")))
	{
		bHasTarget = ChainingBulletTrace(HitResults, StartPoint, BulletDirection, NumPellets, 
									 ConeHalfAngleDegrees, MaxRange, QueryParams);
	}
	else
	{
		bHasTarget = NormalBulletTrace(HitResults, StartPoint, BulletDirection, NumPellets, 
									   ConeHalfAngleDegrees, MaxRange, QueryParams);
	}
    
	return bHasTarget;
}

bool UGA_FireWeapon::NormalBulletTrace(TArray<FHitResult>& HitResults, const FVector& StartPoint,
	const FVector& BulletDirection, float NumPellets, float ConeHalfAngleDegrees, float MaxRange,
	const FCollisionQueryParams& QueryParams)
{
	bool bHasTarget = false;
	for (int32 i = 0; i < NumPellets; ++i)
	{
		FVector ShootDirection = FMath::VRandCone(BulletDirection, FMath::DegreesToRadians(ConeHalfAngleDegrees));
		FVector EndPoint = StartPoint + (ShootDirection * MaxRange);
        
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, NORMAL_TRACE, QueryParams);
        
		if (bHit)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.Component->GetName());
			DrawDebugSphere(GetWorld(), HitResult.Location, 2.0f, 30, FColor::Red, false, 2.0f);
            
			HitResults.Add(HitResult);
			bHasTarget = true;
		}
	}
	return bHasTarget;
}

bool UGA_FireWeapon::PiercingBulletTrace(TArray<FHitResult>& HitResults, const FVector& StartPoint,
	const FVector& BulletDirection, float NumPellets, float ConeHalfAngleDegrees, float MaxRange,
	const FCollisionQueryParams& QueryParams)
{
	bool bHasTarget = false;
	for (int32 i = 0; i < NumPellets; ++i)
	{
		FVector ShootDirection = FMath::VRandCone(BulletDirection, FMath::DegreesToRadians(ConeHalfAngleDegrees));
		FVector EndPoint = StartPoint + (ShootDirection * MaxRange); // Trace distance
		TArray<FHitResult> PiercingHitResults;
		bool bHit = GetWorld()->LineTraceMultiByChannel(PiercingHitResults, StartPoint, EndPoint, PIERCING_TRACE, QueryParams);
		if (bHit)
		{
			TArray<FHitResult> ValidHits;
			for (const FHitResult& HitResult : PiercingHitResults)
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.Component->GetName());
				DrawDebugSphere(GetWorld(), HitResult.Location, 2.0f, 30, FColor::Red, false, 2.0f);
				if (!IsDuplicateHit(ValidHits, HitResult.GetActor()))
				{
					ValidHits.Add(HitResult);
				}
			}
			for (const FHitResult& HitResult : ValidHits)
			{
				HitResults.Add(HitResult);
			}
			
			bHasTarget = true;
		}
	}

	return bHasTarget;
}

bool UGA_FireWeapon::ChainingBulletTrace(TArray<FHitResult>& HitResults, const FVector& StartPoint,
	const FVector& BulletDirection, float NumPellets, float ConeHalfAngleDegrees, float MaxRange,
	const FCollisionQueryParams& QueryParams)
{
	bool bHasTarget = false;
	float SphereRadius = 100.0f;
	
	for (int32 i = 0; i < NumPellets; ++i)
	{
		FVector ShootDirection = FMath::VRandCone(BulletDirection, FMath::DegreesToRadians(ConeHalfAngleDegrees));
		FVector EndPoint = StartPoint + (ShootDirection * MaxRange);
        
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, NORMAL_TRACE, QueryParams);
        
		if (bHit)
		{
			TArray<FHitResult> ValidHits;
			for (int32 j = 0; i < 5; ++i)
			{
				
				TArray<FOverlapResult> OverlapResults;
				bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
				OverlapResults,
				HitResult.Location,               // FVector center
				FQuat::Identity, // No rotation
				NORMAL_TRACE,
				FCollisionShape::MakeSphere(SphereRadius),
				QueryParams);
				for (const FOverlapResult& OverlapResult : OverlapResults)
				{
					bool bIsTargetable = GetWorld()->LineTraceSingleByChannel(HitResult, HitResult.Location, OverlapResult.GetActor()->GetActorLocation(), NORMAL_TRACE, QueryParams);
					if (bIsTargetable)
					{
						if (!IsDuplicateHit(ValidHits, HitResult.GetActor()))
						{
							UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.Component->GetName());
							DrawDebugSphere(GetWorld(), HitResult.Location, 2.0f, 30, FColor::Red, false, 2.0f);
							ValidHits.Add(HitResult);
						}
					}
				}
				
				
			}
			
			for (const FHitResult& ValidHit : ValidHits)
			{
				HitResults.Add(ValidHit);
			}
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
