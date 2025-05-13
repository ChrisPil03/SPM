// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"

#include "DashComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "InteractInterface.h"
#include "GunBase.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "DiveGameMode.h"
#include "PlayerAttributeSet.h"
#include "ScoreManagerComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->bUsePawnControlRotation = true;
	GunComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Component"));
	GunComponent->SetupAttachment(CameraComponent);
	
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

	EquippedGun->SetActorHiddenInGame(false);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UPlayerAttributeSet::GetHealthAttribute()
	).AddUObject(this, &APlayerCharacter::OnCurrentHealthChanged);

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
void APlayerCharacter::Interact()
{
	FHitResult HitResult;

	if (!IsInRange(HitResult))
	{
		return;
	}
	// Make sure the actor was hit and implements the interact interface
	AActor *HitActor = HitResult.GetActor();

	if (!HitActor && !HitActor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		return;
	}
	// Cast to the interface and check if it's valid
	IInteractInterface *Interface = Cast<IInteractInterface>(HitActor);

	if (Interface == nullptr)
	{
		return;
	}

	Interface->Interact(this);
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
		ReleasedTrigger();
	}

	EquippedGun = NewWeapon;

	if (EquippedGun)
	{
		EquippedGun->SetActorHiddenInGame(false);
	}
}


void APlayerCharacter::ChangeEquippedGun(int32 WeaponSlot)
{
	EquipWeapon(Guns[WeaponSlot]);
	
}

bool APlayerCharacter::IsInRange(FHitResult &HitResult) const
{
	AController *PlayerController = GetController();
	if (PlayerController == nullptr)
	{
		return false;
	}
	FVector Location;
	FRotator Rotation;

	PlayerController->GetPlayerViewPoint(Location, Rotation);

	FVector EndPoint = Location + Rotation.Vector() * InteractRange;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->SweepSingleByChannel(
	HitResult,
	Location,
	EndPoint,
	FQuat::Identity,
	ECC_GameTraceChannel2,
	FCollisionShape::MakeSphere(InteractSphereRadius),
	Params
	);

	if (bHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.Location, InteractSphereRadius, 12, FColor::Red, false, 1);
	}
		
	
	return bHit;
	
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

void APlayerCharacter::ResetCharacterPosition()
{
	AGameModeBase *GameMode = GetWorld()->GetAuthGameMode();
	if (GameMode)
	{
		APlayerController *PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		AActor *PlayerStartActor = GameMode->FindPlayerStart(PC);
		SetActorLocationAndRotation(PlayerStartActor->GetActorLocation(), PlayerStartActor->GetActorRotation());
	}
}
