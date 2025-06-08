// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.generated.h"

class APlayerCharacter;
class APlayerLocationDetection;
DECLARE_MULTICAST_DELEGATE_OneParam(FPerformDelegate, AInteractableObject*)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractDelegate, AInteractableObject*, Object);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableEvent);

UCLASS()
class COOLGANG_API AInteractableObject : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	AInteractableObject();

protected:
	virtual void BeginPlay() override;

	UStaticMeshComponent* GetMesh() const { return Mesh; }
public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void Interact(AActor* Interactor) override;
	virtual bool CanInteract() override { return bCanInteractWith; }
	virtual void ResetInteractable();
	bool GetCanInteractWith() const { return bCanInteractWith && bPlayerInProximity; }

	UFUNCTION(BlueprintCallable)
	void SetCanInteractWith(bool const bNewState);

	UFUNCTION(BlueprintCallable)
	void ShowInteractableOutline(const bool bNewState);

	UPROPERTY(BlueprintAssignable)
	FInteractDelegate InteractDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnInteractableEvent OnActivated;

	UPROPERTY(BlueprintAssignable)
	FOnInteractableEvent OnReset;
	
private:
	void BindInteractTrigger();
	void InteractionTriggerEnter(APlayerLocationDetection* Trigger);
	void InteractionTriggerExit(APlayerLocationDetection* Trigger);
	void FindInteractTrigger();
	
	UPROPERTY(EditAnywhere, Category = "Interact")
	bool bCanInteractWith;

	UPROPERTY(VisibleAnywhere, Category = "Interact")
	bool bPlayerInProximity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Interact")
	APlayerLocationDetection* InteractTrigger;
};
