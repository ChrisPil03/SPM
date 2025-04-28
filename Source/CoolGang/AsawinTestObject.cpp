// Fill out your copyright notice in the Description page of Project Settings.


#include "AsawinTestObject.h"

// Sets default values
AAsawinTestObject::AAsawinTestObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAsawinTestObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAsawinTestObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAsawinTestObject::Interact(AActor* Interactor)
{
	if (GiveGun.IsBound())
	{
		GiveGun.Execute();
	}
	
	
	UE_LOG(LogTemp, Warning, TEXT("Interact with %s"), *this->GetName());
	
}

