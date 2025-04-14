// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

UCLASS()
class COOLGANG_API AGun : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGun();
	void Shoot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	// maybe need to change later
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	//can change for other system if needed
	UPROPERTY(EditAnywhere, Category=Gameplay)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	USoundBase* MuzzleSound;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	USoundBase* ImpactSound;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	FVector MuzzleOffset;
	
	UPROPERTY(EditAnywhere, Category=Gameplay)
	float MaxRange = 1000;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditAnywhere)
	float Damage{10};

	bool GunTrace(FHitResult& Hit, FVector& ShotDirection);
	AController* GetOwnerController() const;
};
