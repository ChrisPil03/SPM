// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "DashComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "InteractInterface.h"
#include "GunBase.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->bUsePawnControlRotation = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	GunComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Component"));
	GunComponent->SetupAttachment(CameraComponent);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	EquippedGun = GetWorld()->SpawnActor<AGunBase>(GunClass);
	EquippedGun->AttachToComponent(GunComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

	//EquippedGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket"));
	EquippedGun->SetOwner(this);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HealthComponent->GetCurrentHealth() <= 0)
	{
		Die();
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
	AActor* HitActor = HitResult.GetActor();
        
	if (!HitActor && !HitActor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		return;
	}
			// Cast to the interface and check if it's valid
	IInteractInterface* Interface = Cast<IInteractInterface>(HitActor);
            
	if (Interface == nullptr)
	{
		return;
	}
				
	Interface->Interact(this); 
	
}

void APlayerCharacter::PullTrigger()
{
	EquippedGun->StartFire();
}
void APlayerCharacter::ReleasedTrigger()
{
	EquippedGun->StopFire();
}

void APlayerCharacter::ReloadCurrentGun()
{
	EquippedGun->Reload();
}

void APlayerCharacter::Dash()
{
	UDashComponent* DashComponent = Cast<UDashComponent>(GetComponentByClass(UDashComponent::StaticClass()));
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
	
	
	DashComponent->Dash(Direction);
	UE_LOG(LogTemp, Warning, TEXT("Dash Distance: a"));
}


bool APlayerCharacter::IsInRange(FHitResult& HitResult) const
{
	
		AController* PlayerController  = GetController();
		if (PlayerController == nullptr)
		{
			return false;
		}
		FVector Location;
		FRotator Rotation;
	
		PlayerController->GetPlayerViewPoint(Location, Rotation);
	
	
		FVector EndPoint = Location + Rotation.Vector() * InteractRange;
		DrawDebugLine(GetWorld(), Location, EndPoint, FColor::Red, false, 2);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		return GetWorld()->LineTraceSingleByChannel(HitResult, Location, EndPoint, ECC_GameTraceChannel2, Params);
}

void APlayerCharacter::Die()
{
	//Destroy();
}

bool APlayerCharacter::IsDead() const
{
	return bDead;
}


