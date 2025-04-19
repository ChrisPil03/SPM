// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.generated.h"

DECLARE_DELEGATE_OneParam(FPerformDelegate, AInteractableObject*);

UCLASS()
class COOLGANG_API AInteractableObject : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	virtual void Interact(AActor* Interactor) override;
	virtual void ResetInteractable() { bInteractedWith = false; }
	bool GetInteractedWith() const { return bInteractedWith; };
	void SetInteractFunction(const FPerformDelegate& NewFunction) { PerformDelegate = NewFunction; };

private:
	bool bInteractedWith;
	
	FPerformDelegate PerformDelegate;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;
};
