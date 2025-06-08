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
	if (GetCanInteractWith())
	{
		if (InteractDelegate.IsBound())
		{
			InteractDelegate.Broadcast(this);
		}
		SetCanInteractWith(false);
	}
}

void AInteractableObject::ResetInteractable()
{
	SetCanInteractWith(false);

	if (OnReset.IsBound())
	{
		OnReset.Broadcast();
	}
}

void AInteractableObject::SetCanInteractWith(bool const bNewState)
{
	if (bNewState)
	{
		if (OnActivated.IsBound())
		{
			OnActivated.Broadcast();
		}
		if (bPlayerInProximity)
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(
				UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				Player->ShowInteractPrompt(true);
			}
		}
	}else
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(
			UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			Player->ShowInteractPrompt(false);
		}
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
		InteractTrigger->AddOnTriggerEnterFunction(this, &AInteractableObject::InteractionTriggerEnter);
		InteractTrigger->AddOnTriggerExitFunction(this, &AInteractableObject::InteractionTriggerExit);
	}
}

void AInteractableObject::InteractionTriggerEnter(APlayerLocationDetection* Trigger)
{
	bPlayerInProximity = true;
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(
	UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Player->AddInteractable(this);
	}
}

void AInteractableObject::InteractionTriggerExit(APlayerLocationDetection* Trigger)
{
	bPlayerInProximity = false;
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(
	UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Player->ClearInteractable(this);
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
