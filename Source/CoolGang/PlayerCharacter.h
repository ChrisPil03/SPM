// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Attackable.h"
#include "GameFramework/Character.h"
#include "HealthComponent.h"
#include "PlayerCharacter.generated.h"

class UScoreManagerComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerTakeDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentHealthChangedDelegate, float, CurrentHealth);

class AGunBase;
struct FOnAttributeChangeData;
UCLASS(BlueprintType)
class COOLGANG_API APlayerCharacter : public ACharacter, public IAttackable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure)
	bool IsDead() const;
	
	UFUNCTION(BlueprintCallable)
	void Interact();

	UFUNCTION(BlueprintCallable)
	void PullTrigger();

	UFUNCTION(BlueprintCallable)
	void ReleasedTrigger();

	UFUNCTION(BlueprintCallable)
	void ActivateGunAbility();

	UFUNCTION(BlueprintCallable)
	void ReloadCurrentGun();
	
	UFUNCTION(BlueprintCallable)
	void Dash();

	UFUNCTION(BlueprintCallable)
	void ChangeEquippedGun(int32 WeaponSlot);

	UFUNCTION(BlueprintCallable)
	AGunBase* GetEquippedGun() const{ return EquippedGun;};
	void EquipWeapon(AGunBase* NewWeapon);

	UPROPERTY(BlueprintAssignable, Category="Events");
	FOnPlayerTakeDamage OnPlayerTakeDamage;

	void Die();

private:
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), Category="Component")
	class UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, Category=Interact)
	float InteractRange = 200;

	UPROPERTY(EditAnywhere, Category=Interact)
	float InteractSphereRadius = 10;

	bool IsInRange(FHitResult& HitResult) const;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	bool bDead;

	UPROPERTY(EditAnywhere)
	float Health;

	UPROPERTY(EditAnywhere)
	float MaxHealth;
	
	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Pistol;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Rifle;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Shotgun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TArray<AGunBase*> Guns;

	void GiveGun(const TSubclassOf<AGunBase>& GunClass);

	UPROPERTY()
	AGunBase* EquippedGun;
	

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), Category="Component")
	UStaticMeshComponent* GunComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	class UAbilitySystemComponent *AbilitySystemComponent;

	UPROPERTY()
	const class UPlayerAttributeSet* PlayerAttributeSet;

	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_InitPlayerStats;
	void InitPlayerStats();
	
	void OnCurrentHealthChanged(const FOnAttributeChangeData& Data) const;

	UPROPERTY(BlueprintAssignable, Category = "Gameplay")
	FOnCurrentHealthChangedDelegate OnCurrentHealthChangedDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite ,meta = (AllowPrivateAccess = "true"), Category = "Component")
	UScoreManagerComponent* ScoreManagerComponent;
};
