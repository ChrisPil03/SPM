// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	SetRootComponent(Mesh);
	GunEffectSpawnPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunSpawnPoint"));
	GunEffectSpawnPoint->SetupAttachment(Mesh);
	
}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60.0f / FireRate;
	AmmoInMag = MagazineSize;
	
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
			HitResult.GetActor()->TakeDamage(Damage, DamageEvent, OwnerController, this);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *HitResult.GetActor()->GetActorLabel());
			BlinkDebug(HitResult);
		}
	}
	
	AmmoInMag--;
	SetAmmoInMagText(AmmoInMag);
	
	UE_LOG(LogTemp, Warning, TEXT(" Pew!! %d"), AmmoInMag);
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
	DrawDebugLine(GetWorld(), Location, EndPoint, FColor::Red, false, 0.5f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	
	return GetWorld()->LineTraceSingleByChannel(Hit, Location, EndPoint, ECC_GameTraceChannel1, Params);
	
}
void AGunBase::StartFire()
{
	if (!CanFire()) return;
	
	bIsFiring = true;
	if (GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle))
	{
		return;
	}
	
	
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
	bCanFire = false;
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, [this]()
	{
		AmmoInMag = MagazineSize;
		bCanFire = true;
		SetAmmoInMagText(AmmoInMag);
	}, TimeBetweenShots, false);
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
