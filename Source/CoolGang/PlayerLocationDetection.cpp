// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerLocationDetection.h"

#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"

// Sets default values
APlayerLocationDetection::APlayerLocationDetection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger Box Component"));
	RootComponent = TriggerBox;
}

// Called when the game starts or when spawned
void APlayerLocationDetection::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerLocationDetection::OnBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &APlayerLocationDetection::OnEndOverlap);
}

void APlayerLocationDetection::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	if (Cast<APlayerCharacter>(OtherActor))
	{
		if (OnTriggerEnter.IsBound())
		{
			OnTriggerEnter.Broadcast(this);
		}
	}
}

void APlayerLocationDetection::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	if (Cast<APlayerCharacter>(OtherActor))
	{
		if (OnTriggerExit.IsBound())
		{
			OnTriggerExit.Broadcast(this);
		}
	}
}
