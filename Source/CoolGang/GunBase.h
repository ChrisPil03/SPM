// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

UCLASS()
class COOLGANG_API AGunBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGunBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// maybe need to change later
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	//can change for other system if needed
	UPROPERTY(EditAnywhere, Category = "Gun | Effect" )
	class UNiagaraSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Gun | Sound" )
	USoundBase* MuzzleSound;

	
	UPROPERTY(EditAnywhere, Category = "Gun | Sound" )
	USoundBase* ImpactSound;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactEffect;
	
	UPROPERTY(EditAnywhere, Category=Gameplay)
	FVector MuzzleOffset;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* GunEffectSpawnPoint;

	

	/////////////////  Gun property  //////////////////////////
	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float Damage{10};

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float FireRate{10};

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float MaxRange = 1000;

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float ReloadTime = 2;

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	int MagazineSize = 30;

	UPROPERTY(EditAnywhere, Category = "Gun | Stat ")
	int AmmoInMag = 30;

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	int Recoil = 2;
	
	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	bool bIsAutomatic = false;

	
	bool GunTrace(FHitResult& Hit, FVector& ShotDirection);
	AController* GetOwnerController() const;

	FTimerHandle FireTimerHandle;
	float TimeBetweenShots;
	bool bCanFire = true;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void StartFire();
	virtual void StopFire();
	virtual void Fire();
	virtual void Reload();
	
	bool CanFire() const;
	bool bIsFiring = false;

	UFUNCTION(BlueprintCallable)
	int GetMagazineSize(){return MagazineSize;};

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetAmmoInMagText(int Ammo);

	UFUNCTION(BlueprintCallable)
	int GetAmmoInMag(){return AmmoInMag;};
	UFUNCTION(BlueprintCallable)
	int GetRecoil(){return Recoil;};
	UFUNCTION(BlueprintCallable)
	bool GetIsAutomatic(){return bIsAutomatic;};
private:
	void BlinkDebug(FHitResult& h);
	FTimerHandle BlinkTimerHandle;
	
};


