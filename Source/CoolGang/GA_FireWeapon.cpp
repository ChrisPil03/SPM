// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_FireWeapon.h"
#include "AbilitySystemComponent.h"
#include "GunBase.h"
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
	UE_LOG(LogTemp, Warning, TEXT("Ammo before shoot: %f"), CurrentAmmo );
	// Check that at least 1 bullet is available
	return CurrentAmmo >= 1;
}

void UGA_FireWeapon::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("Shoot"));
	
	APlayerCharacter* OwningPlayerCharacter = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()->GetOwner());
	AGunBase*  EquippedWeapon = OwningPlayerCharacter->GetEquippedGun();
	if (EquippedWeapon)
	{
		EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
		switch (WeaponType)
		{
		case EWeaponType::Shotgun:
			PelletsFire();
			break;
		case EWeaponType::Rifle:
			SingleBulletFire();
			break;
		case EWeaponType::Pistol:
			SingleBulletFire();
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Unknown Weapon Type!"));
			break;
		}
	}
}

void UGA_FireWeapon::SingleBulletFire()
{
	UE_LOG(LogTemp, Warning, TEXT("Shoot") );
	
	FHitResult HitResult;
	SingleTrace(HitResult);
	
	FGameplayAbilityTargetDataHandle TargetData;
	FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	NewTargetData->HitResult = HitResult;
	TargetData.Add(NewTargetData);
	
	OnTargetDataReady(TargetData);
}

void UGA_FireWeapon::PelletsFire()
{


	TArray<FHitResult> HitResults;
	MultiTrace(HitResults);
	
	FGameplayAbilityTargetDataHandle TargetData;
	for (int32 i = 0; i < HitResults.Num(); i++)
	{
		FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
		NewTargetData->HitResult = HitResults[i];
		TargetData.Add(NewTargetData);
	}
	
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

bool UGA_FireWeapon::SingleTrace(FHitResult& Hit)
{
	FVector StartPoint;
	FRotator Rotation;
	GetTraceStartLocationAndRotation(StartPoint, Rotation);
	const FVector BulletDirection = Rotation.Vector();
	FVector EndPoint = StartPoint + (BulletDirection * 20000000);  // range 
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwningActorFromActorInfo());
	QueryParams.AddIgnoredActor(GetOwningActorFromActorInfo()->GetOwner());

	if ( GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, ECC_GameTraceChannel1, QueryParams))
	{
		DrawDebugSphere(GetWorld(), Hit.Location, 2.0f, 12,FColor::Red, false, 2.0f);
		
		//BlinkDebug(Hit);
		return true;
	}
		return false;

}

bool UGA_FireWeapon::MultiTrace(TArray<FHitResult>& HitResults)
{
	bool bHasTarget = false;
	FVector StartPoint;
	FRotator Rotation;
	GetTraceStartLocationAndRotation(StartPoint, Rotation);
	const FVector BulletDirection = Rotation.Vector();
	const UAbilitySystemComponent* ASC = GetActorInfo().AbilitySystemComponent.Get();
	const UWeaponAttributeSet* Attributes = ASC->GetSet<UWeaponAttributeSet>();
	float NumPellets = Attributes->GetPellets();
	const float ConeHalfAngleDegrees = 5.0f;

	for (int32 i = 0; i < NumPellets; ++i)
	{
		FVector ShootDirection = FMath::VRandCone(BulletDirection, FMath::DegreesToRadians(ConeHalfAngleDegrees));
		FVector EndPoint = StartPoint + (ShootDirection * 10000.0f); // Trace distance

		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwningActorFromActorInfo());
		QueryParams.AddIgnoredActor(GetOwningActorFromActorInfo()->GetOwner()); // Ignore self

		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, ECC_GameTraceChannel1, QueryParams);

		if (bHit)
		{
			HitResults.Add(Hit);
			DrawDebugSphere(GetWorld(), Hit.Location, 2.0f, 12,FColor::Red, false, 2.0f);
			//BlinkDebug(Hit);
			
			bHasTarget = true;
		}
	}

	return bHasTarget;
}


void UGA_FireWeapon::BlinkDebug(FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (HitActor == nullptr)
	{
		return;
	}

	UStaticMeshComponent* MeshComponent = HitActor->FindComponentByClass<UStaticMeshComponent>();

	if (MeshComponent == nullptr)
	{
		return;		
	}
		// Save original material
		UMaterialInterface* OriginalMaterialInterface = MeshComponent->GetMaterial(0);
		FString AssetPath = OriginalMaterialInterface->GetPathName();
		
		// Load red material
		if (UMaterialInterface* RedMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Assets/Materials/M_Debug.M_Debug")))
		{
			MeshComponent->SetMaterial(0, RedMaterial);

			// Start a timer to revert the material after 0.2 seconds
			
			FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this, AssetPath, MeshComponent]()
			{
				UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *AssetPath );
				MeshComponent->SetMaterial(0, Material);
			});
	
			GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, TimerDelegate, 0.1, false);
		}
	
}

