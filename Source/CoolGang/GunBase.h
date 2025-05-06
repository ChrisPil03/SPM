// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GunBase.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Shotgun UMETA(DisplayName = "Shotgun"),
	Rifle UMETA(DisplayName = "Rifle"),
	Pistol UMETA(DisplayName = "Pistol"),
	// Add other weapon types as needed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoCountChangedDelegate, float, Ammo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagazineSizeChangedDelegate, float, Ammo);

class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;
UCLASS()
class COOLGANG_API AGunBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGunBase();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// maybe need to change later
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere)
	USceneComponent* Root;
	///   Sound   ///
	UPROPERTY(EditAnywhere,BlueprintReadOnly , Category = "Gun | Sound" )
	USoundCue* BulletSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun | Sound" )
	USoundCue* PullTriggerSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun | Sound" )
	USoundCue* ReloadSound;
	

	///   Effect   ///
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactEffect;
	
	UPROPERTY(EditAnywhere, Category = "Gun | Effect" )
	class UNiagaraSystem* MuzzleFlashEffect;
	
	UPROPERTY(EditAnywhere, Category=Gameplay)
	USceneComponent* MuzzlePosition;
	
	UPROPERTY(EditAnywhere, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> CameraShakeClass;
	
	/////////////////  Gun property  //////////////////////////
	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float Pellets{7};
	
	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float Damage{10};

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float FireRate{10};

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float MaxRange {1000};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun | Stat")
	float ReloadTime {2};

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	int MagazineSize {30};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun | Stat ")
	int AmmoCount {30};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun | Stat ")
	float BulletSpreadAngle {0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun | Stat")
	bool bIsAutomatic = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun | Stat")
	float TimeBetweenShots;
	
	void InitWeaponStats();
	void GiveAbilities();
	
	UPROPERTY()
	const class UWeaponAttributeSet* WeaponAttributeSet;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent *AbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<UGameplayEffect> GE_InitWeaponStats;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> FireAbilityClass;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> ReloadAbilityClass;
	
public:	
	
	
	UFUNCTION(BlueprintImplementableEvent)
	void StartFire();
	
	UFUNCTION(BlueprintImplementableEvent)
	void StopFire();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Fire();
	
	UFUNCTION(BlueprintImplementableEvent)
	void Reload();

	UFUNCTION(BlueprintCallable)
	int GetMagazineSize(){return MagazineSize;};

	UFUNCTION(BlueprintCallable)
	bool IsAutomatic(){return bIsAutomatic;};

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetAmmoCountText(float Ammo);

	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetMaxAmmoText(float Ammo);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType;

	EWeaponType GetWeaponType() const { return WeaponType; }
	
	
	UFUNCTION(BlueprintCallable)
	TSubclassOf<UCameraShakeBase> GetCameraShakeClass() const
	{
		return CameraShakeClass;
	}
	
	void Initialize();
protected:
	UFUNCTION()
	void CalculateTimeBetweenShots(float NewFireRate);

	void OnFireRateChanged(const FOnAttributeChangeData& Data);

	void OnAmmoCountChanged(const FOnAttributeChangeData& Data) const;

	UPROPERTY(BlueprintAssignable, Category = "Gameplay")
	FOnAmmoCountChangedDelegate OnAmmoCountChangedDelegate;

	void OnMagazineSizeChanged(const FOnAttributeChangeData& Data) const;

	UPROPERTY(BlueprintAssignable, Category = "Gameplay")
	FOnMagazineSizeChangedDelegate OnMagazineSizeChangedDelegate;
	
};



