// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_FireWeapon.h"
#include "AbilitySystemComponent.h"
#include "WeaponAttributeSet.h"

#include "PlayerCharacter.h"

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
	UE_LOG(LogTemp, Warning, TEXT("Check cost for shoot") );
	float CurrentAmmo = Attributes->GetAmmoCount();
	UE_LOG(LogTemp, Warning, TEXT("Ammo: %f"), CurrentAmmo );
	// Check that at least 1 bullet is available
	return CurrentAmmo >= 1;
}

void UGA_FireWeapon::Fire()
{

	UE_LOG(LogTemp, Warning, TEXT("Shoot") );
	
	FHitResult HitResult;
	SingleTrace(HitResult);
	if (HitResult.GetActor() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor is nullptr") );
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetActorNameOrLabel());
	}
	FGameplayAbilityTargetDataHandle TargetData;
	FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	NewTargetData->HitResult = HitResult;

	TargetData.Add(NewTargetData);
	OnRangedWeaponTargetDataReady(TargetData);
}

bool UGA_FireWeapon::SingleTrace(FHitResult& Hit)
{
	
	APawn* OwningPawn = Cast<APawn>(GetOwningActorFromActorInfo()); 
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
	FVector StartPoint;
	FRotator Rotation;
	
	OwnerController->GetPlayerViewPoint(StartPoint, Rotation);
	const FVector BulletDirection = Rotation.Vector();
	
	FVector EndPoint = StartPoint + Rotation.Vector() * 20000;
	
	//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 0.5f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningPawn);
	
	return GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, ECC_GameTraceChannel1, Params);

	

	
}

