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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageChangedDelegate, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoCountChangedDelegate, float, AmmoCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagazineSizeChangedDelegate, float, MagazineSize);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireRateChangedDelegate, float, FireRate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReloadTimeChangedDelegate, float, ReloadTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUltimateReadyDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUltimateStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUltimateEndDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadCancelDelegate);

class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UNiagaraSystem;

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

protected:
	// maybe need to change later
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere)
	USceneComponent* Root;

	///   Sound   ///
	UPROPERTY(EditAnywhere,BlueprintReadOnly , Category = "Gun | Sound" )
	USoundCue* BulletSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun | Sound" )
	USoundCue* ReloadSound;

	
	///   Effect   ///
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun | Effect" )
	UNiagaraSystem* ImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun | Effect" )
	UNiagaraSystem* NormalImpactEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun | Effect" )
	UNiagaraSystem* UltimateImpactEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun | Effect" )
	UNiagaraSystem* MuzzleFlashEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun | Effect" )
	UNiagaraSystem* UltimateMuzzleFlashEffect;
	
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
	
	UPROPERTY()
	const class UWeaponAttributeSet* WeaponAttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent *AbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<UGameplayEffect> GE_InitWeaponStats;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> FireAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> ReloadAbilityClass;

	/// Ultimate ///
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Gun | Ultimate", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> UltimateAbilityClass;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Gun | Ultimate", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> UltimateEffectClass;

	
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly )
	UMaterialInstance* PickupMat;

public:
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable )
	void StartFire();
	virtual void StartFire_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopFire();
	virtual void StopFire_Implementation();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Fire();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ActivateAbility();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintCallable)
	void CancelReload();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnEquipped();

	void Initialize();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Gameplay, meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzlePosition;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun | Ultimate", meta = (AllowPrivateAccess = "true"))
	float UltimateDuration {0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun | Ultimate", meta = (AllowPrivateAccess = "true"))
	float UltimateCooldown {0};
	
protected:
	UFUNCTION(BlueprintCallable)
	void OnUltimateReady();

	UFUNCTION(BlueprintCallable)
	void OnUltimateStart();

	UFUNCTION(BlueprintCallable)
	void OnUltimateEnd();
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite )
	bool bIsUltimateReady;

	UFUNCTION(BlueprintCallable)
	bool IsAutomatic(){return bIsAutomatic;};

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetAmmoCountText(float Ammo);
	
	void InitWeaponStats();
	void GiveAbilities();
	
	UFUNCTION()
	void CalculateTimeBetweenShots(float NewFireRate);

	void OnFireRateChanged(const FOnAttributeChangeData& Data);

	UPROPERTY(BlueprintAssignable, Category = "Weapon Stats")
	FOnFireRateChangedDelegate OnFireRateChangedDelegate;

	void OnAmmoCountChanged(const FOnAttributeChangeData& Data) const;

	UPROPERTY(BlueprintAssignable, Category = "Weapon Stats")
	FOnAmmoCountChangedDelegate OnAmmoCountChangedDelegate;

	void OnMagazineSizeChanged(const FOnAttributeChangeData& Data) const;

	UPROPERTY(BlueprintAssignable, Category = "Weapon Stats")
	FOnMagazineSizeChangedDelegate OnMagazineSizeChangedDelegate;

	void OnDamageChanged(const FOnAttributeChangeData& Data) const;
	
	UPROPERTY(BlueprintAssignable, Category = "Weapon Stats")
	FOnDamageChangedDelegate OnDamageChangedDelegate;

	void OnReloadTimeChanged(const FOnAttributeChangeData& Data) const;
	UPROPERTY(BlueprintAssignable, Category = "Weapon Stats")
	FOnReloadTimeChangedDelegate OnReloadTimeChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "WeaponUltimate")
	FOnUltimateReadyDelegate OnUltimateReadyDelegate;

	UPROPERTY(BlueprintAssignable, Category = "WeaponUltimate")
	FOnUltimateStartDelegate OnUltimateStartDelegate;

	UPROPERTY(BlueprintAssignable, Category = "WeaponUltimate")
	FOnUltimateEndDelegate OnUltimateEndDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnReloadCancelDelegate OnReloadCancelDelegate;
};


