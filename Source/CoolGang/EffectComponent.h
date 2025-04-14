// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EffectComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOLGANG_API UEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEffectComponent();
	void SetEffectDuration(float Second){ EffectDuration = Second; };

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	float EffectDuration{0};
	FTimerHandle EffectTimer;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ApplyEffect(AActor* Actor);
private:
	
};
