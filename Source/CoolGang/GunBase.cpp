// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "EnemyAI.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"

// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	SetRootComponent(Mesh);
	MuzzlePosition = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle Position"));
	MuzzlePosition->SetupAttachment(Mesh);
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60.0f / FireRate;
	AmmoInMag = MagazineSize;
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitWeaponStats();

	if (AbilitySystemComponent && FireAbilityClass)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(FireAbilityClass, /*Level=*/1));
	}
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AGunBase::InitWeaponStats()
{
	if (AbilitySystemComponent)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitWeaponStats, 1.f, Context);

		if (Spec.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Init Spec is valid"));

			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.AmmoCount"), 20);
			//Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), 20);

			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Init Spec is NOT valid"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AbilitySystemComponent is null in BeginPlay!"));
	}
}

void AGunBase::Fire()
{
	if (!CanFire())
	{
		StopFire();
		
		return;
		
	}
	if (!bIsFiring)
	{
		return; // Skip firing if we're no longer supposed to be firing
	}

	if (BulletSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), BulletSound, GetActorLocation());
	}
	
	if (MuzzleFlash)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlash,
			RootComponent,
			NAME_None, // No socket
			FVector::ZeroVector, // Offset
			FRotator::ZeroRotator, // Rotation
			EAttachLocation::SnapToTargetIncludingScale,
			true // Auto destroy
		);
	}
	//UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh, TEXT("MuzzleFlashSocket"));
	//UGameplayStatics::SpawnSoundAttached(MuzzleSound, Mesh, TEXT("MuzzleFlashSocket"));
	FHitResult HitResult;
	FVector ShotDirection;
	
	if(GunTrace(HitResult, ShotDirection))
	{
		
		//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.Location, ShotDirection.Rotation());
		//UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, Hit.Location);
		if (HitResult.GetActor())
		{
			FPointDamageEvent DamageEvent(Damage, HitResult, ShotDirection, nullptr);
			AController* OwnerController = GetOwnerController();
			//HitResult.GetActor()->TakeDamage(Damage, DamageEvent, OwnerController, this);
			
			if (Cast<AEnemyAI>(HitResult.GetActor()))
			{
				
				UAbilitySystemComponent* EnemyASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitResult.GetActor());
				UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
				
				BlinkDebug(HitResult);
				
				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GE_OnHit, 1.0f, Context);

				if (SpecHandle.IsValid())
				{
					// Set the caller magnitude BEFORE applying
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Damage.Type.Bullet")), -Damage);

					// Apply to target
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), EnemyASC);
				}
				
			}
			
		}
	}

	StartRecoil();
	AmmoInMag--;
	SetAmmoInMagText(AmmoInMag);
	
	//UE_LOG(LogTemp, Warning, TEXT(" Pew!! %d"), AmmoInMag);
}

bool AGunBase::GunTrace(FHitResult& Hit, FVector& ShotDirection)
{
	AController* OwnerController = GetOwnerController();
	if (OwnerController == nullptr)
	{
		return false;
	}
	FVector Location;
	FRotator Rotation;
	
	OwnerController->GetPlayerViewPoint(Location, Rotation);

	ShotDirection = -Rotation.Vector();
	
	FVector EndPoint = Location + Rotation.Vector() * MaxRange;
	//DrawDebugLine(GetWorld(), Location, EndPoint, FColor::Red, false, 0.5f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	
	return GetWorld()->LineTraceSingleByChannel(Hit, Location, EndPoint, ECC_GameTraceChannel1, Params);
	
}

void AGunBase::StartFire()
{
	if (bIsReloading)
	{return;}
	if (!CanFire())
	{
		if (PullTriggerSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), PullTriggerSound, GetActorLocation());
		}
		return;
	}
	
	
	if (GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle))
	{
		return;
	}
	
	bIsFiring = true;
	if (bIsAutomatic)
	{
			Fire(); // Immediate first shot
			GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &AGunBase::Fire, TimeBetweenShots, true);
	}
	else
	{
		Fire();
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, FTimerDelegate::CreateLambda([this](){}), TimeBetweenShots, false);
		bIsFiring = false;
	}
}

 void AGunBase::StopFire()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	
}

 void AGunBase::Reload()
{
	if (bIsReloading)
	{
		return;
	}
	
	bIsReloading = true;
	bCanFire = false;
	StopFire();
	
	
	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());
	}
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, [this]()
	{
		AmmoInMag = MagazineSize;
		bCanFire = true;
		bIsReloading = false;
		SetAmmoInMagText(MagazineSize);

		
	}, ReloadTime, false);
}

void AGunBase::StartRecoil()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetOwnerController()))
	{
		PlayerController->PlayerCameraManager->StartCameraShake(CameraShakeClass, 1.0f);
	}
}


bool AGunBase::CanFire() const
{
	return bCanFire && AmmoInMag > 0;
}


AController* AGunBase::GetOwnerController() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return nullptr;
	}
	return OwnerPawn->GetController();

	
}

void AGunBase::BlinkDebug(FHitResult& HitResult)
{
	if (UStaticMeshComponent* MeshComponent = HitResult.GetActor()->FindComponentByClass<UStaticMeshComponent>())
	{
		// Save original material
		UMaterialInterface* OrginalMaterialInterface = MeshComponent->GetMaterial(0);
		FString AssetPath = OrginalMaterialInterface->GetPathName();
		
		// Load red material
		
		UMaterialInterface* RedMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Assets/Materials/M_Debug.M_Debug"));
		if (RedMaterial)
		{
			MeshComponent->SetMaterial(0, RedMaterial);

			// Start a timer to revert the material after 0.2 seconds
			
			FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this, AssetPath, MeshComponent]()
			{
				UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *AssetPath );
				MeshComponent->SetMaterial(0, Material);
			});
	
			GetOwner()->GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, TimerDelegate, 0.1, false);
		}
	}
}

