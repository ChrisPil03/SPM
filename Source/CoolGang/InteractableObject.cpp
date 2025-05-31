#include "InteractableObject.h"

#include "PlayerCharacter.h"
#include "PlayerLocationDetection.h"
#include "Kismet/GameplayStatics.h"

AInteractableObject::AInteractableObject()
{
	PrimaryActorTick.bCanEverTick = true;
	bCanInteractWith = false;
	bPlayerInProximity = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();
	FindInteractTrigger();
	BindInteractTrigger();
}

void AInteractableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInteractableObject::Interact(AActor* Interactor)
{
	if (InteractDelegate.IsBound())
	{
		InteractDelegate.Broadcast(this);
	}
	if (GetCanInteractWith())
	{
		
		if (!PerformDelegate.IsBound())
		{
			return;
		}
		PerformDelegate.Broadcast(this);
		SetCanInteractWith(false);

		if (APlayerCharacter* Player = Cast<APlayerCharacter>(
		UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			Player->ClearAvailableInteractable(this);
		}
	}
}

void AInteractableObject::ResetInteractable()
{
	SetCanInteractWith(false);
}

void AInteractableObject::SetCanInteractWith(bool const bNewState)
{
	if (!bNewState)
	{
		InteractionNotAvailable(nullptr);
	}
	bCanInteractWith = bNewState;
	ShowInteractableOutline(bNewState);
}

void AInteractableObject::ShowInteractableOutline(const bool bNewState)
{
	Mesh->bRenderCustomDepth = bNewState;
	Mesh->MarkRenderStateDirty();
}

void AInteractableObject::BindInteractTrigger()
{
	if (InteractTrigger)
	{
		InteractTrigger->AddOnTriggerEnterFunction(this, &AInteractableObject::InteractionAvailable);
		InteractTrigger->AddOnTriggerExitFunction(this, &AInteractableObject::InteractionNotAvailable);
	}
}

void AInteractableObject::InteractionAvailable(APlayerLocationDetection* Trigger)
{
	if (bCanInteractWith)
	{
		bPlayerInProximity = true;

		if (APlayerCharacter* Player = Cast<APlayerCharacter>(
			UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			Player->AddAvailableInteractable(this);
		}
	}
}

void AInteractableObject::InteractionNotAvailable(APlayerLocationDetection* Trigger)
{
	if (bCanInteractWith)
	{
		bPlayerInProximity = false;
		
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(
			UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			Player->ClearAvailableInteractable(this);
		}
	}
}

void AInteractableObject::FindInteractTrigger()
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (AActor* Actor : AttachedActors)
	{
		if (APlayerLocationDetection* Trigger = Cast<APlayerLocationDetection>(Actor))
		{
			InteractTrigger = Trigger;
			break;
		}
	}
}
