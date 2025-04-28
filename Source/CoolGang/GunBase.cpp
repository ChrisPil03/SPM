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
	AmmoCount = MagazineSize;
	
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}


void AGunBase::Initialize()
{
	InitAbilitySystemComponent();
	InitWeaponStats();
}


void AGunBase::InitWeaponStats()
{
	if (AbilitySystemComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AbilitySystemComponent is null in BeginPlay!"));
		return;
	}
	
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitWeaponStats, 1.f, Context);

	if (Spec.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Init Gun Spec is valid"));

		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.AmmoCount"), AmmoCount);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.MagazineSize"), MagazineSize);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), Damage);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.FireRate"), FireRate);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Pellets"), Pellets);
		WeaponAttributeSet = AbilitySystemComponent->GetSet<UWeaponAttributeSet>();
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Init Gun Spec is NOT valid"));
	}
}

void AGunBase::InitAbilitySystemComponent()
{
	if (GetOwner() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GunOwner is nullptr"));
		return;
	}
	AbilitySystemComponent->InitAbilityActorInfo(GetOwner(), this);

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

