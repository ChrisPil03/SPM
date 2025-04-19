// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DashComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOLGANG_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDashComponent();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void Dash();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:

	UPROPERTY(EditAnywhere)
	float DashForce = 0.0f;

	UPROPERTY(EditAnywhere)
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere)
	float DashDistance = 0.0f;

	UPROPERTY(EditAnywhere)
	float StopTime = 0.0f;
	
	UPROPERTY()
	 ACharacter* OwnerCharacter;

	FTimerHandle CooldownTimer;
	FTimerHandle DashTimer;
	
	bool bIsDashing;
	FVector StartLocation;
	float OriginalGroundFriction;
	
	void CheckToReset();

	void Reset();
};
