// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "SphereTriggerComponent.generated.h"

class AObjectiveDownloadStation;
/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOLGANG_API USphereTriggerComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	USphereTriggerComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	UPROPERTY(EditAnywhere)
	FName TagToCheckFor;

	AObjectiveDownloadStation* Objective;
	
	AActor* GetAcceptableActor() const;
};
