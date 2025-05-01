// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "GunBase.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Shotgun UMETA(DisplayName = "Shotgun"),
	Rifle UMETA(DisplayName = "Rifle"),
	Pistol UMETA(DisplayName = "Pistol"),
	// Add other weapon types as needed
};


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

	///   Sound   ///
	UPROPERTY(EditAnywhere, Category = "Gun | Sound" )
	USoundBase* BulletSound;

	UPROPERTY(EditAnywhere, Category = "Gun | Sound" )
	USoundBase* PullTriggerSound;

	UPROPERTY(EditAnywhere, Category = "Gun | Sound" )
	USoundBase* ReloadSound;
	
	UPROPERTY(EditAnywhere, Category = "Gun | Sound" )
	USoundBase* ImpactSound;

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

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float MinRecoil {0.5f};

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float MaxRecoil {1.0f};

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float RecoilDuration {0.3f};

	UPROPERTY(EditAnywhere, Category = "Gun | Stat")
	float RecoilExponent {2.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun | Stat")
	bool bIsAutomatic = false;

	
	AController* GetOwnerController() const;

	FTimerHandle FireTimerHandle;
	FTimerHandle ReloadTimerHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun | Stat")
	float TimeBetweenShots;
	bool bCanFire = true;
	bool bIsRecoiling  = false;
	bool bIsReloading  = false;


	void InitWeaponStats();
	void GiveAbilities();
	

	float ElapsedTime =  0.0f;
	 

	float StartPitch = 0.0f;
	float TargetPitch = 0.0f;
	
	UPROPERTY()
	const class UWeaponAttributeSet* WeaponAttributeSet;

	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UAbilitySystemComponent *AbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_InitWeaponStats;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UGameplayAbility> FireAbilityClass;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UGameplayAbility> ReloadAbilityClass;

	UPROPERTY()
	FGameplayAbilitySpecHandle FireHandle;

	UPROPERTY()
	FGameplayAbilitySpecHandle ReloadHandle;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void StartFire();
	
	UFUNCTION(BlueprintImplementableEvent)
	void StopFire();

	UFUNCTION(BlueprintImplementableEvent)
	void StopReload();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Fire();
	
	UFUNCTION(BlueprintImplementableEvent)
	void Reload();
	void StartRecoil();
	
	bool CanFire() const;
	bool bIsFiring = false;

	UFUNCTION(BlueprintCallable)
	int GetMagazineSize(){return MagazineSize;};

	UFUNCTION(BlueprintCallable)
	bool IsAutomatic(){return bIsAutomatic;};

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetAmmoCountText(float Ammo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxAmmoText(float Ammo);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType;

	EWeaponType GetWeaponType() const { return WeaponType; }
	
	class UAbilitySystemComponent* GetAbilitySystemComponent() const
	{
		return AbilitySystemComponent;
	}
	
	void Initialize();
};



