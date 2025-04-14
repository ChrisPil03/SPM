// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "PickupComponent.generated.h"

/**
 * 
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THELYINGCAKE_API UPickupComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	bool bHasBeenCollected;
	
	UPROPERTY(EditAnywhere, Category = "Pickup")
	TSubclassOf<UEffectComponent> EffectClass;
	
	UPROPERTY(EditAnywhere, Category = "Pickup")
	class UNiagaraSystem* PicUpParticle;
	
	UPROPERTY(EditAnywhere, Category = "Pickup")
	class USoundBase* SoundEffect;
	UPROPERTY(EditAnywhere, Category = "Pickup" )
	float EffectDuration{0};
	
	void ApplyEffectToActor(AActor* ActorToApply);
	
	UFUNCTION()
	void OnPickup(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};