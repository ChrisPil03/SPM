// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HealthComponent.h"
#include "PlayerCharacter.generated.h"

class AGunBase;
UCLASS()
class COOLGANG_API APlayerCharacter : public ACharacter
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
	void ReloadCurrentGun();
	
	UFUNCTION(BlueprintCallable)
	void Dash();

	UFUNCTION(BlueprintCallable)
	AGunBase* GetEquippedGun() const{ return EquippedGun;};

private:
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), Category="Component")
	class UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, Category=Gameplay)
	float InteractRange = 100;

	bool IsInRange(FHitResult& HitResult) const;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	bool bDead;
	
	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AGunBase> GunClass;

	UPROPERTY()
	AGunBase* EquippedGun;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), Category="Component")
	UStaticMeshComponent* GunComponent;


	void Die();
};
