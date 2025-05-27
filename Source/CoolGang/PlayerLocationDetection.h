// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerLocationDetection.generated.h"

class UBoxComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTriggerEnterDelegate, APlayerLocationDetection*)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTriggerExitDelegate, APlayerLocationDetection*)

UCLASS()
class COOLGANG_API APlayerLocationDetection : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerLocationDetection();

protected:
	virtual void BeginPlay() override;

public:	
	template <typename T>
	void AddOnTriggerEnterFunction(T* Object, void (T::*Func)(APlayerLocationDetection*))
	{
		OnTriggerEnter.AddUObject(Object, Func);
	}
	
	template <typename T>
	void AddOnTriggerExitFunction(T* Object, void (T::*Func)(APlayerLocationDetection*))
	{
		OnTriggerExit.AddUObject(Object, Func);
	}

	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	
	void FindPlayerAlreadyInsideDetectionZone(APawn* ControlledPawn);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	UBoxComponent* TriggerBox;
	
	FOnTriggerEnterDelegate OnTriggerEnter;
	FOnTriggerExitDelegate OnTriggerExit;
};
