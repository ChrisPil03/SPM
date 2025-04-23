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

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) return;

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	
	if (Avatar == nullptr)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Shoot") );

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Avatar);
	
	if (PlayerCharacter == nullptr)
	{
		return;
	}
	FHitResult HitResult;
	SingleTrace(HitResult);
	UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetActorNameOrLabel());

	
	//UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	//if (AbilitySystemComponent == nullptr)
	//{
	//	return;
	//}
	
	//AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(GE_AmmoConsumed), 1, AbilitySystemComponent->MakeEffectContext());
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_FireWeapon::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const UWeaponAttributeSet* Attributes = ASC->GetSet<UWeaponAttributeSet>();
	UE_LOG(LogTemp, Warning, TEXT("Check cost for shoot") );
	float CurrentAmmo = Attributes->GetAmmoCount();

	// Check that at least 1 bullet is available
	return CurrentAmmo >= 1;
}

bool UGA_FireWeapon::SingleTrace(FHitResult& Hit)
{
	ACharacter* const CharacterAvatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	AController* OwnerController = CharacterAvatar->GetController();
	if (OwnerController == nullptr)
	{
		return false;
	}
	FVector StartPoint;
	FRotator Rotation;
	
	OwnerController->GetPlayerViewPoint(StartPoint, Rotation);
	const FVector BulletDirection = -Rotation.Vector();
	
	FVector EndPoint = StartPoint + Rotation.Vector() * 20000;
	
	//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 0.5f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CharacterAvatar);
	
	return GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, ECC_GameTraceChannel1, Params);
	
}

