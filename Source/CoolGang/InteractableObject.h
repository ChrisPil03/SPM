// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.generated.h"

class APlayerCharacter;
class APlayerLocationDetection;
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
	bool GetCanInteractWith() const { return bCanInteractWith && bPlayerInProximity; }
	void SetCanInteractWith(bool const bNewState);
	void SetInteractFunction(const FPerformDelegate& NewFunction) { PerformDelegate = NewFunction; }

	template <typename T>
	void SetOnInteractFunction(T* Object, void (T::*Func)(AInteractableObject*))
	{
		PerformDelegate.AddUObject(Object, Func);
	}

private:
	void ShowInteractableOutline(const bool bNewState);
	void BindInteractTrigger();
	void InteractionAvailable(APlayerLocationDetection* Trigger);
	void InteractionNotAvailable(APlayerLocationDetection* Trigger);
	void FindInteractTrigger();
	
	UPROPERTY(VisibleAnywhere, Category = "Interact")
	bool bCanInteractWith;

	UPROPERTY(VisibleAnywhere, Category = "Interact")
	bool bPlayerInProximity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Interact")
	APlayerLocationDetection* InteractTrigger;
};
