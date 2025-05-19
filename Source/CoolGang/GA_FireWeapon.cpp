// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_FireWeapon.h"
#include "AbilitySystemComponent.h"
#include "GunBase.h"
#include "WeaponAttributeSet.h"
#include "EnemyAI.h"

#include "PlayerCharacter.h"
#include "Engine/OverlapResult.h"
#define PIERCING_TRACE ECC_GameTraceChannel6
#define NORMAL_TRACE ECC_GameTraceChannel1
#define CHAINING_TRACE ECC_GameTraceChannel7
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
		UE_LOG(LogTemp, Warning, TEXT("PiercingActive"));
		bHasTarget = PiercingBulletTrace(HitResults, StartPoint, BulletDirection, NumPellets, 
										 ConeHalfAngleDegrees, MaxRange, QueryParams);
	}
	else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.ChainingActive")))
	{
		UE_LOG(LogTemp, Warning, TEXT("ChainingActive"));
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
		FVector ShootDirection = FMath::VRandCone(BulletDirection,
				 FMath::DegreesToRadians(ConeHalfAngleDegrees));
		FVector EndPoint = StartPoint + (ShootDirection * MaxRange);
        
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, NORMAL_TRACE, QueryParams);
        
		if (bHit)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.Component->GetName());
			DrawImpactPointDeBug(HitResult.Location);
			if (HitResult.GetActor()->IsA(AEnemyAI::StaticClass()))
			{
				HitResults.Add(HitResult);
				bHasTarget = true;
			}
			else
			{
				SpawnImpactEffect(HitResult);
			}
			
		
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
		FVector ShootDirection = FMath::VRandCone(
			BulletDirection,
		FMath::DegreesToRadians(ConeHalfAngleDegrees)
		);
		FVector EndPoint = StartPoint + (ShootDirection * MaxRange);
		TArray<FHitResult> PiercingHitResults;
		bool bHit = GetWorld()->LineTraceMultiByChannel(PiercingHitResults,
		StartPoint, EndPoint, PIERCING_TRACE, QueryParams);
		if (bHit)
		{
			TArray<FHitResult> ValidHits;
			for (const FHitResult& HitResult : PiercingHitResults)
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.Component->GetName());
				DrawImpactPointDeBug(HitResult.Location);
				if (HitResult.GetActor()->IsA(AEnemyAI::StaticClass()))
				{
					if (!IsDuplicateHit(ValidHits, HitResult.GetActor()))
					{
						ValidHits.Add(HitResult);
					}
				}
				else
				{
					SpawnImpactEffect(HitResult);
				}
			}
			if (!ValidHits.IsEmpty())
			{
				HitResults.Append(ValidHits);
				bHasTarget = true;
			}
		}
	}

	return bHasTarget;
}

bool UGA_FireWeapon::ChainingBulletTrace(TArray<FHitResult>& HitResults, const FVector& StartPoint,
    const FVector& BulletDirection, float NumPellets, float ConeHalfAngleDegrees, float MaxRange,
    const FCollisionQueryParams& QueryParams)
{
    bool bHasTarget = false;
    const float SphereRadius = 1000.0f;
    const int32 MaxChainDepth = 4; // Limit how many times a bullet can chain
    
    FCollisionQueryParams OverlapQueryParams;
    OverlapQueryParams.AddIgnoredActor(GetOwningActorFromActorInfo());
    OverlapQueryParams.AddIgnoredActor(GetOwningActorFromActorInfo()->GetOwner());
	
    for (int32 i = 0; i < NumPellets; ++i)
    {
        FVector ShootDirection = FMath::VRandCone(BulletDirection, FMath::DegreesToRadians(ConeHalfAngleDegrees));
        FVector EndPoint = StartPoint + (ShootDirection * MaxRange);
    	
        FHitResult InitialHit;
        bool bHit = GetWorld()->LineTraceSingleByChannel(InitialHit, StartPoint, EndPoint, NORMAL_TRACE, QueryParams);
    	DrawImpactPointDeBug(InitialHit.Location);
       
        if (!bHit)
        {
            continue;
        }

    	if (!InitialHit.GetActor()->IsA(AEnemyAI::StaticClass()))
    	{
    		SpawnImpactEffect(InitialHit);
    		continue;
    	}
    	
        TArray<FHitResult> PelletHits;
        ProcessHitChain(InitialHit, PelletHits, MaxChainDepth, SphereRadius, OverlapQueryParams);
    	
        if (!PelletHits.IsEmpty())
        {
            HitResults.Append(PelletHits);
            bHasTarget = true;
        }
    }
    
    return bHasTarget;
}

void UGA_FireWeapon::ProcessHitChain(const FHitResult& InitialHit, TArray<FHitResult>& ChainHits, 
    int32 MaxChainDepth, float SphereRadius, FCollisionQueryParams& OverlapQueryParams)
{
    // Add initial hit to results
    ChainHits.Add(InitialHit);
    
    // Track actors we've already hit to avoid duplicates
    TArray<AActor*> HitActors;
    if (InitialHit.GetActor())
    {
        HitActors.Add(InitialHit.GetActor());
        OverlapQueryParams.AddIgnoredActor(InitialHit.GetActor());
    }
	
    FVector CurrentHitLocation = InitialHit.Location;
    
    // Chain process (limited by MaxChainDepth)
    for (int32 ChainIndex = 0; ChainIndex < MaxChainDepth - 1; ChainIndex++)
    {
        // Visual debugging for the sphere overlap
        DrawDebugSphere(GetWorld(), CurrentHitLocation, SphereRadius, 30, FColor::Blue, false, 2.0f);
        
        // Find nearby potential targets
        TArray<FOverlapResult> OverlapResults;
        bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
            OverlapResults,
            CurrentHitLocation,
            FQuat::Identity,
            CHAINING_TRACE,
            FCollisionShape::MakeSphere(SphereRadius),
            OverlapQueryParams);
            
        if (!bHasOverlap || OverlapResults.IsEmpty())
        {
            break; // No more targets to chain to
        }
        
        // Find the closest valid target
        float ClosestDistanceSq = FLT_MAX;
        AActor* ClosestActor = nullptr;
        FHitResult ClosestHitResult;
        
        for (const FOverlapResult& OverlapResult : OverlapResults)
        {
            AActor* PotentialTarget = OverlapResult.GetActor();
            if (!PotentialTarget || HitActors.Contains(PotentialTarget))
            {
                continue;
            }
            
            // Check if it's an enemy
            AEnemyAI* Enemy = Cast<AEnemyAI>(PotentialTarget);
            if (!Enemy)
            {
                continue;
            }
            
            // Get target position
            FVector TargetOrigin;
            FVector BoxExtent;
            Enemy->GetActorBounds(true, TargetOrigin, BoxExtent);
            
            // Check visibility with line trace
            FHitResult VisibilityHit;
            bool bIsVisible = GetWorld()->LineTraceSingleByChannel(
                VisibilityHit, 
                CurrentHitLocation, 
                TargetOrigin, 
                NORMAL_TRACE, 
                OverlapQueryParams);
                
            // Only consider if line trace hits the intended target
            if (bIsVisible && VisibilityHit.GetActor() == Enemy)
            {
                float DistanceSq = FVector::DistSquared(CurrentHitLocation, TargetOrigin);
                
                UE_LOG(LogTemp, Warning, TEXT("Potential chain target: %s; Distance: %f"), 
                    *Enemy->GetActorNameOrLabel(), FMath::Sqrt(DistanceSq));
                
                // Visualize potential chain
                //DrawDebugLine(
                //	GetWorld(), CurrentHitLocation, TargetOrigin,
               // 	FColor::Green, false, 2.0f
               // 	);
                
                if (DistanceSq < ClosestDistanceSq)
                {
                    ClosestDistanceSq = DistanceSq;
                    ClosestActor = Enemy;
                    ClosestHitResult = VisibilityHit;
                }
            }
        }
        
        // If no valid target was found, end the chain
        if (!ClosestActor)
        {
            break;
        }
        
        // Add the new hit to our results
        ChainHits.Add(ClosestHitResult);
        HitActors.Add(ClosestActor);
        OverlapQueryParams.AddIgnoredActor(ClosestActor);
        
        // Visualize the selected chain
        DrawDebugLine(
        	GetWorld(), CurrentHitLocation, ClosestHitResult.Location,
        	FColor::Yellow, false, 2.0f
        	);
        UE_LOG(LogTemp, Warning, TEXT("Chained to target: %s"), *ClosestActor->GetActorNameOrLabel());
        
        // Update current position for next chain
        CurrentHitLocation = ClosestHitResult.Location;
    }
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

void UGA_FireWeapon::DrawImpactPointDeBug(const FVector& Location) const
{
	DrawDebugSphere(
				GetWorld(),
				Location,
				2.0f,
				30,
				FColor::Red,
				false,
				2.
			);
}

void UGA_FireWeapon::SpawnImpactEffect(const FHitResult& HitResult) const
{
	FGameplayCueParameters CueParams;
	FGameplayEffectContextHandle EffectContext = GetActorInfo().AbilitySystemComponent.Get()->MakeEffectContext();
	EffectContext.AddHitResult(HitResult);
	CueParams.EffectContext = EffectContext;
	// This will play the cue on the local ASC (usually the shooter)
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.ApplyDamageToEnemy")), CueParams);
	}
	
}
