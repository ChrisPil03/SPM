// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FPerformDelegate, AInteractableObject*)

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

	UStaticMeshComponent* GetMesh() const { return Mesh; }

	FPerformDelegate PerformDelegate;
public:
	virtual void Tick(float DeltaTime) override;
	virtual void Interact(AActor* Interactor) override;
	virtual void ResetInteractable() { bCanInteractWith = false; }
	bool GetCanInteractWith() const { return bCanInteractWith; }
	void SetCanInteractWith(bool const bState) { bCanInteractWith = bState; }
	void SetInteractFunction(const FPerformDelegate& NewFunction) { PerformDelegate = NewFunction; }

private:
	bool bCanInteractWith;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;
};
