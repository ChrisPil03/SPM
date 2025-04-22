#include "ObjectiveDefendGenerator.h"
#include "HealthComponent.h"
#include "InteractableObject.h"

AObjectiveDefendGenerator::AObjectiveDefendGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	SetIsTimeBased(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(RootComponent);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
}

void AObjectiveDefendGenerator::BeginPlay()
{
	Super::BeginPlay();
	BindControlPanel();
	BindDeathFunction();
	BindCompletionFunction();
}

void AObjectiveDefendGenerator::RegisterControlPanelInteraction(AInteractableObject* InteractableObject)
{
	StartObjective();
}

void AObjectiveDefendGenerator::CompleteObjective()
{
	Super::CompleteObjective();
}

void AObjectiveDefendGenerator::ResetObjective()
{
	Super::ResetObjective();
	HealthComponent->ResetHealthToMax();
}

void AObjectiveDefendGenerator::BindControlPanel()
{
	if (ControlPanel)
	{
		FPerformDelegate Delegate;
		Delegate.AddUObject(this, &AObjectiveDefendGenerator::RegisterControlPanelInteraction);
		ControlPanel->SetInteractFunction(Delegate);
	}
}

void AObjectiveDefendGenerator::BindDeathFunction()
{
	FDeathDelegate Delegate;
	Delegate.BindUObject(this, &AObjectiveDefendGenerator::ResetObjective);
	HealthComponent->SetDeathFunction(Delegate);
}

void AObjectiveDefendGenerator::BindCompletionFunction()
{
	FTimerCompletionDelegate Delegate;
	Delegate.BindUObject(this, &AObjectiveDefendGenerator::CompleteObjective);
	GetProgressTimer().SetCompletionDelegate(Delegate);
}
