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
	if (!GetIsActive())
	{
		return;
	}
	StartObjective();
	InitiateQuarantine();
}

void AObjectiveDefendGenerator::InitiateQuarantine()
{
	UE_LOG(LogTemp, Warning, TEXT("Quarantine initiated"));
}

bool AObjectiveDefendGenerator::CanNotTakeDamage()
{
	return !GetIsActive() || GetIsComplete() || GetIsFailed();
}

void AObjectiveDefendGenerator::CompleteObjective()
{
	Super::CompleteObjective();
}

void AObjectiveDefendGenerator::ResetObjective()
{
	Super::ResetObjective();
	HealthComponent->ResetHealthToMax();
	ControlPanel->SetCanInteractWith(true);
}

float AObjectiveDefendGenerator::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (CanNotTakeDamage())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot take damage"));
		return 0;
	}
	DamageAmount = FMath::Min(DamageAmount, HealthComponent->GetCurrentHealth());
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
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
	Delegate.BindUObject(this, &Super::FailObjective);
	HealthComponent->SetDeathFunction(Delegate);
}

void AObjectiveDefendGenerator::BindCompletionFunction()
{
	FTimerCompletionDelegate Delegate;
	Delegate.BindUObject(this, &AObjectiveDefendGenerator::CompleteObjective);
	GetProgressTimer().SetCompletionDelegate(Delegate);
}
