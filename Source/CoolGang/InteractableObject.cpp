// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableObject.h"

AInteractableObject::AInteractableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bCanInteractWith = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInteractableObject::Interact(AActor* Interactor)
{
	if (bCanInteractWith)
	{
		UE_LOG(LogTemp, Display, TEXT("Interacted with object: %s"), *GetName());
		
		if (!PerformDelegate.IsBound())
		{
			UE_LOG(LogTemp, Warning, TEXT("No function bound to perform on interact!"));
			return;
		}
		PerformDelegate.Broadcast(this);
		SetCanInteractWith(false);
	}
}

void AInteractableObject::SetCanInteractWith(bool const bNewState)
{
	bCanInteractWith = bNewState;
	ShowInteractableOutline(bNewState);
}

void AInteractableObject::ShowInteractableOutline(const bool bNewState)
{
	Mesh->bRenderCustomDepth = bNewState;
	Mesh->MarkRenderStateDirty();
}
