// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "JumpPadTriggerBoxComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COOLGANG_API UJumpPadTriggerBoxComponent : public UBoxComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

protected:
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
					   bool bFromSweep, const FHitResult& SweepResult);
private:
 UPROPERTY(EditAnywhere)
	float ZForce = 0.f;
	
};
