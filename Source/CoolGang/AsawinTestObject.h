// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractInterface.h"
#include "AsawinTestObject.generated.h"

DECLARE_DELEGATE(FGiveGun)
UCLASS()
class COOLGANG_API AAsawinTestObject : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAsawinTestObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintImplementableEvent)
	void Interact(AActor* Interactor);
	virtual bool CanInteract() override { return false; }

};

