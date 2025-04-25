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
	void SetOnTriggerEnter(const FOnTriggerEnterDelegate& NewDelegate) { OnTriggerEnter = NewDelegate; }
	void SetOnTriggerExit(const FOnTriggerEnterDelegate& NewDelegate) { OnTriggerExit = NewDelegate; }

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	UBoxComponent* TriggerBox;
	
	FOnTriggerEnterDelegate OnTriggerEnter;
	FOnTriggerEnterDelegate OnTriggerExit;
};
