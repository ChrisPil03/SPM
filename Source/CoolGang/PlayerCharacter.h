// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Attackable.h"
#include "GameFramework/Character.h"
#include "HealthComponent.h"
#include "PlayerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerTakeDamage);

class AGunBase;
UCLASS(BlueprintType)
class COOLGANG_API APlayerCharacter : public ACharacter, public IAttackable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

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
	

private:
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), Category="Component")
	class UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, Category=Gameplay)
	float InteractRange = 200;

	bool IsInRange(FHitResult& HitResult) const;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	bool bDead;
	
	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Pistol;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Rifle;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Shotgun;

	UPROPERTY()
	AGunBase* EquippedGun;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), Category="Component")
	UStaticMeshComponent* GunComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	class UAbilitySystemComponent *AbilitySystemComponent;

	void Die();

	UFUNCTION(BlueprintCallable, meta=(AllowPrivateAccess="true"))
	void ResetCharacterPosition();

	UFUNCTION(BlueprintCallable, meta=(AllowPrivateAccess="true"))
	void ResetCharacterHealth();

	UPROPERTY()
	class UPlayerAttributeSet* PlayerAttributeSet;

	UPROPERTY()
	TArray<AGunBase*> Guns;


};
