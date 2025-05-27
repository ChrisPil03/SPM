// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"

#include "DashComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "InteractInterface.h"
#include "GunBase.h"
#include "AbilitySystemComponent.h"
#include "DiveGameMode.h"
#include "PlayerAttributeSet.h"
#include "ScoreManagerComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->bUsePawnControlRotation = true;
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArmComponent->SetupAttachment(CameraComponent);
	
	GunComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Component"));
	GunComponent->SetupAttachment(SpringArmComponent);
	
	ScoreManagerComponent = CreateDefaultSubobject<UScoreManagerComponent>(TEXT("Score Manager Component"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitPlayerStats();
	GiveGun(Pistol);
	GiveGun(Shotgun);
	GiveGun(Rifle);

	bDead = false;
	
	EquippedGun->SetActorHiddenInGame(false);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UPlayerAttributeSet::GetHealthAttribute()
	).AddUObject(this, &APlayerCharacter::OnCurrentHealthChanged);

	UE_LOG(LogTemp, Display, TEXT("Begin play for character"));
	if (OnPlayerConstructed.IsBound())
	{
		OnPlayerConstructed.Broadcast();
	}
	
}

TArray<USphereComponent*> APlayerCharacter::GetMovementNodes()
{
	return EnemyTargetSpheres;
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::PullTrigger()
{
	if (!EquippedGun)
	{
		return;
	}
	EquippedGun->StartFire();
}

void APlayerCharacter::ReleasedTrigger()
{
	if (!EquippedGun)
	{
		return;
	}
	EquippedGun->StopFire();
}

void APlayerCharacter::ActivateGunAbility()
{
	EquippedGun->ActivateAbility();
}

void APlayerCharacter::ReloadCurrentGun()
{
	if (!EquippedGun)
	{
		return;
	}
	ReleasedTrigger();
	EquippedGun->Reload();
}

void APlayerCharacter::Dash()
{
	UDashComponent *DashComponent = Cast<UDashComponent>(GetComponentByClass(UDashComponent::StaticClass()));
	if (DashComponent == nullptr)
	{
		return;
	}

	FVector Direction = GetVelocity();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}
	Direction = Direction.GetSafeNormal();

	DashComponent->Dash();
}


void APlayerCharacter::EquipWeapon(AGunBase* NewWeapon)
{
	if (EquippedGun)
	{
		EquippedGun->SetActorHiddenInGame(true);
		EquippedGun->CancelReload();
		UE_LOG(LogTemp, Warning, TEXT("CancelReload"));
		ReleasedTrigger();
	}

	EquippedGun = NewWeapon;

	if (EquippedGun)
	{
		EquippedGun->SetActorHiddenInGame(false);
		EquippedGun->OnEquipped();
	}
}


void APlayerCharacter::ChangeEquippedGun(int32 WeaponSlot)
{
	EquipWeapon(Guns[WeaponSlot]);
	
}

void APlayerCharacter::Die()
{
	if (!IsDead())
	{
		bDead = true;
		ADiveGameMode *GameMode = GetWorld()->GetAuthGameMode<ADiveGameMode>();
		if (GameMode != nullptr)
		{
			GameMode->PlayerKilled(this);
			SaveScore();
		}
	}
}

bool APlayerCharacter::IsDead() const
{
	return bDead;
}

void APlayerCharacter::OnCurrentHealthChanged(const FOnAttributeChangeData& Data) const
{
	float NewCurrentHealth = Data.NewValue;
	OnCurrentHealthChangedDelegate.Broadcast(NewCurrentHealth);
}

void APlayerCharacter::GiveGun(const TSubclassOf<AGunBase>& GunClass)
{
	EquippedGun = GetWorld()->SpawnActor<AGunBase>(GunClass);
	EquippedGun->AttachToComponent(GunComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	EquippedGun->SetOwner(this);
	EquippedGun->SetActorHiddenInGame(true);
	EquippedGun->Initialize();
	Guns.Add(EquippedGun);
}

void APlayerCharacter::InitPlayerStats()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitPlayerStats, 1.f, Context);

		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Health"), Health);
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			
		}
	}	

}

// ----------------- Interact ----------------- //
// -------------------------------------------- //

void APlayerCharacter::SetAvailableInteractable(IInteractInterface* Interactable)
{
	AvailableInteractable = Interactable;
	
	if (InteractWidget)
	{
		InteractWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void APlayerCharacter::ClearAvailableInteractable(const IInteractInterface* Interactable)
{
	if (AvailableInteractable == Interactable)
	{
		AvailableInteractable = nullptr;

		if (InteractWidget)
		{
			InteractWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void APlayerCharacter::Interact()
{
	if (AvailableInteractable)
	{
		AvailableInteractable->Interact(this);
	}
}