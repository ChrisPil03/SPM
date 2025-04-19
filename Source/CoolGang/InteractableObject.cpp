// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableObject.h"

AInteractableObject::AInteractableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bInteractedWith = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableObject::Interact(AActor* Interactor)
{
	if (!bInteractedWith)
	{
		UE_LOG(LogTemp, Display, TEXT("Interacted with object: %s"), *GetName());
		
		if (!PerformDelegate.IsBound())
		{
			UE_LOG(LogTemp, Error, TEXT("No function bound to perform on interact!"));
			return;
		}
		PerformDelegate.ExecuteIfBound(this);
		bInteractedWith = true;
	}
}