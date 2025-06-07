// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Attackable.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter.generated.h"

class IInteractInterface;
class UScoreManagerComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerConstructed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerTakeDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentHealthChangedDelegate, float, CurrentHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FONCurrentWeaponChanged, int, WeaponIndex);
class AGunBase;
struct FOnAttributeChangeData;
UCLASS(BlueprintType)
class COOLGANG_API APlayerCharacter : public ACharacter, public IAttackable
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	
protected:
	virtual void BeginPlay() override;
	
	bool IsPlayerConsideredNearGround(float& OutGroundDistance) const;
	
	bool IsNodeInsideSolid(const USphereComponent* Node) const;
	
	bool GetNodeDistanceToGround(const USphereComponent* Node, float& OutDistanceToGround) const;

	UPROPERTY(EditAnywhere, Category = "AI Targeting|Movement Nodes")
	float PlayerNearGroundThreshold; // Max distance for player to be "near ground" (e.g., your jump height)

	UPROPERTY(EditAnywhere, Category = "AI Targeting|Movement Nodes")
	float NodeMaxAerialGroundDistance; // If player is near ground, a node is invalid if its distance to ground is > this

	UPROPERTY(EditAnywhere, Category = "AI Targeting|Movement Nodes")
	float GroundCheckTraceDistance; // How far down to trace for ground checks

	UPROPERTY(EditAnywhere, Category = "AI Targeting|Movement Nodes")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel;

	// Object types to consider "solid" for the IsNodeInsideSolid check
	UPROPERTY(EditAnywhere, Category = "AI Targeting|Movement Nodes")
	TArray<TEnumAsByte<EObjectTypeQuery>> SolidObjectTypes;

	UPROPERTY(EditAnywhere, Category = "AI Targeting|Movement Nodes", meta = (DisplayName = "Draw Debug Traces"))
	bool bDrawMovementNodeDebugTraces;
	
public:
	UFUNCTION()
	virtual TArray<USphereComponent*> GetMovementNodes() override;

	bool HasLineOfSightToNode(const USphereComponent* Node) const;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
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
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnDash();
	
	UFUNCTION(BlueprintCallable)
	void ChangeEquippedGun(int32 WeaponSlot);

	UPROPERTY(BlueprintAssignable, Category = "Events | Player | Weapon")
	FONCurrentWeaponChanged OnWeaponChangedDelegate;

	UFUNCTION(BlueprintCallable)
	AGunBase* GetEquippedGun() const{ return EquippedGun;};
	
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(AGunBase* NewWeapon);

	UPROPERTY(BlueprintAssignable, Category="Events");
	FOnPlayerTakeDamage OnPlayerTakeDamage;

	UPROPERTY(BlueprintAssignable, Category="Events");
	FOnPlayerConstructed OnPlayerConstructed;
	
	UFUNCTION(BlueprintImplementableEvent)
	void SaveScore();
	
	void Die();

	void AddAvailableInteractable(IInteractInterface* Interactable);
	void ClearAvailableInteractable(IInteractInterface* Interactable);

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"), Category="Component")
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), Category="Component")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY()
	TArray<USphereComponent*> EnemyTargetSpheres;
	
	UPROPERTY(EditAnywhere, Category=Interact)
	float InteractRange = 200;

	UPROPERTY(EditAnywhere, Category=Interact)
	float InteractSphereRadius = 10;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	bool bDead;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	float MaxHealth;
	
	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Pistol;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Rifle;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> Shotgun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TArray<AGunBase*> Guns;

	UFUNCTION(BlueprintCallable, Category = "Weapon", meta = (AllowPrivateAccess = "true"), meta = (AllowClasses = "AGunBase"))
	void GiveGun(const TSubclassOf<AGunBase> GunClass);

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
	UPROPERTY(BlueprintAssignable, Category = "Gameplay")
	FOnPlayerDeath OnPlayerDeathDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite ,meta = (AllowPrivateAccess = "true"), Category = "Component")
	UScoreManagerComponent* ScoreManagerComponent;
	
	// IInteractInterface* AvailableInteractable;
	TArray<IInteractInterface*> AvailableInteractions;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UUserWidget* InteractWidget;
};
