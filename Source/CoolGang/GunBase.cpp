// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "WeaponAttributeSet.h"
#include "UObject/ConstructorHelpers.h"
#include "NiagaraComponent.h"


// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
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
}

void AGunBase::Initialize()
{
	GiveAbilities();
	InitWeaponStats();
}

void AGunBase::InitWeaponStats()
{
	if (AbilitySystemComponent == nullptr)
	{
		//UE_LOG(LogTemp, Error, TEXT("AbilitySystemComponent is null in BeginPlay!"));
		return;
	}
	
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitWeaponStats, 1.f, Context);

	if (Spec.IsValid())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Init Gun Spec is valid"));

		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.AmmoCount"), AmmoCount);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.MagazineSize"), MagazineSize);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), Damage);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.FireRate"), FireRate);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.ReloadTime"), ReloadTime);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Pellets"), Pellets);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.BulletSpreadAngle"), BulletSpreadAngle);
		WeaponAttributeSet = AbilitySystemComponent->GetSet<UWeaponAttributeSet>();
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("Init Gun Spec is NOT valid"));
	}
}

void AGunBase::GiveAbilities()
{
	if (GetOwner() == nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("GunOwner is nullptr"));
		return;
	}
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (AbilitySystemComponent && FireAbilityClass)
	{
		FireHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(FireAbilityClass, 1, 0, this));
		ReloadHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(ReloadAbilityClass, 1, 0, this));
	}
	
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
	return bCanFire && AmmoCount > 0;
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