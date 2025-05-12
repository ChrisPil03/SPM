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
	float DashForce = 2000.0f;

	UPROPERTY(EditAnywhere)
	float Cooldown = 1.0f;

	UPROPERTY(EditAnywhere)
	float DashDistance = 200.0f;

	UPROPERTY(EditAnywhere)
	float DashDuration = 1.0f;

	UPROPERTY(EditAnywhere)
	float StopTime = 1.0f;

	UPROPERTY(EditAnywhere)
	float DecelerationSpeed = 1.0f;
	
	UPROPERTY()
	 ACharacter* OwnerCharacter;
	UPROPERTY()
	class UCharacterMovementComponent* CharacterMovement;
	
	FTimerHandle CooldownTimer;
	FTimerHandle DashTimer;
	
	bool bIsDashing;
	bool bShouldDecelerate;
	FVector StartLocation;
	FVector DashDirection;
	float OriginalGroundFriction;
	FVector TargetVelocity;
	FVector CurrentVelocity;
	FVector NewVelocity;
	float DashSpeed;
	bool CheckToReset();

	void Reset();
};
