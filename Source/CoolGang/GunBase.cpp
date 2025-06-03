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
#include "Components/AudioComponent.h"


// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	Mesh->SetupAttachment(Root);
	MuzzlePosition = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle Position"));
	MuzzlePosition->SetupAttachment(Mesh);
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	if (NormalImpactEffect)
	{
		ImpactEffect = NormalImpactEffect;
	}
}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UWeaponAttributeSet::GetFireRateAttribute()
	).AddUObject(this, &AGunBase::OnFireRateChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UWeaponAttributeSet::GetAmmoCountAttribute()
	).AddUObject(this, &AGunBase::OnAmmoCountChanged);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UWeaponAttributeSet::GetMagazineSizeAttribute()
	).AddUObject(this, &AGunBase::OnMagazineSizeChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UWeaponAttributeSet::GetDamageAttribute()
	).AddUObject(this, &AGunBase::OnDamageChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	UWeaponAttributeSet::GetReloadTimeAttribute()
	).AddUObject(this, &AGunBase::OnReloadTimeChanged);
	
}

void AGunBase::CancelReload()
{
	
	FGameplayTagContainer CancelTags;
	CancelTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Reload"));
	AbilitySystemComponent->CancelAbilities(&CancelTags);
	OnReloadCancelDelegate.Broadcast();
	
	UAudioComponent* AudioComponent = FindComponentByClass<UAudioComponent>();
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
}

void AGunBase::OnUltimateReady()
{
	OnUltimateReadyDelegate.Broadcast();
	bIsUltimateReady = true;
}

void AGunBase::OnUltimateStart()
{
	OnUltimateStartDelegate.Broadcast();
}

void AGunBase::OnUltimateEnd()
{
	OnUltimateEndDelegate.Broadcast();
}

void AGunBase::Initialize()
{
	GiveAbilities();
	InitWeaponStats();
}

void AGunBase::CalculateTimeBetweenShots(float NewFireRate)
{
	TimeBetweenShots = 60.0f / NewFireRate;
}

void AGunBase::OnDamageChanged(const FOnAttributeChangeData& Data) const
{
	float NewDamage = Data.NewValue;
	UE_LOG(LogTemp, Warning, TEXT("NEw damage: %f"), NewDamage);
	OnDamageChangedDelegate.Broadcast(NewDamage);
}

void AGunBase::OnReloadTimeChanged(const FOnAttributeChangeData& Data) const
{
	float NewReloadTime = Data.NewValue;
	OnReloadTimeChangedDelegate.Broadcast(NewReloadTime);
}


void AGunBase::OnFireRateChanged(const FOnAttributeChangeData& Data)
{
	float NewFireRate = Data.NewValue;
	CalculateTimeBetweenShots(NewFireRate);
	OnFireRateChangedDelegate.Broadcast(NewFireRate);
}

void AGunBase::OnAmmoCountChanged(const FOnAttributeChangeData& Data) const 
{
	float NewAmmoCount = Data.NewValue;
	OnAmmoCountChangedDelegate.Broadcast(NewAmmoCount);
}

void AGunBase::OnMagazineSizeChanged(const FOnAttributeChangeData& Data) const
{
	float NewMagazineSize = Data.NewValue;
	OnMagazineSizeChangedDelegate.Broadcast(NewMagazineSize);
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
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.MaxRange"), MaxRange);
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
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(FireAbilityClass, 1, 0, this));
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(ReloadAbilityClass, 1, 0, this));
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UltimateAbilityClass, 1, 0, this));
		
	}
	
}

void AGunBase::StopFire_Implementation()
{
	
}


void AGunBase::StartFire_Implementation()
{
	
}