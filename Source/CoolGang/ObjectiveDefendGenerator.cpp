#include "ObjectiveDefendGenerator.h"
#include "HealthComponent.h"
// #include "InteractableObject.h"
#include "Components/CapsuleComponent.h"

AObjectiveDefendGenerator::AObjectiveDefendGenerator() :
	StartDelay(3.f),
	bHalfHealthVoiceLinePlayed(false),
	bLowHealthVoiceLinePlayed(false)
{
	PrimaryActorTick.bCanEverTick = true;
	SetIsTimeBased(true);

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	RootComponent = CapsuleComponent;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(RootComponent);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
}

void AObjectiveDefendGenerator::BeginPlay()
{
	Super::BeginPlay();
	// BindControlPanel();
	BindDeathFunction();
	BindCompletionFunction();
}

void AObjectiveDefendGenerator::StartObjective()
{
	Super::StartObjective();
}

void AObjectiveDefendGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaSeconds);
	}
}

// void AObjectiveDefendGenerator::RegisterControlPanelInteraction(AInteractableObject* InteractableObject)
// {
// 	if (!GetIsActive())
// 	{
// 		return;
// 	}
// 	StartObjective();
// 	InitiateQuarantine();
// }

// void AObjectiveDefendGenerator::InitiateQuarantine()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Quarantine initiated"));
// }

bool AObjectiveDefendGenerator::CannotTakeDamage() const
{
	return !GetIsActive() || GetIsComplete() || GetIsFailed();
}

void AObjectiveDefendGenerator::ResetObjective()
{
	Super::ResetObjective();
	bHalfHealthVoiceLinePlayed = false;
	bLowHealthVoiceLinePlayed = false;
	HealthComponent->ResetHealthToMax();
	//ControlPanel->SetCanInteractWith(true);
}

float AObjectiveDefendGenerator::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (CannotTakeDamage())
	{
		//UE_LOG(LogEngine, Warning, TEXT("Cannot take damage"));
		return 0;
	}
	//UE_LOG(LogEngine, Warning, TEXT("Taking %f damage!"), DamageAmount);
	DamageAmount = FMath::Min(DamageAmount, HealthComponent->GetCurrentHealth());
	if (!bHalfHealthVoiceLinePlayed &&
		HealthComponent->GetCurrentHealth() - DamageAmount <= HealthComponent->GetMaxHealth() / 2)
	{
		EnqueueVoiceLineWithMessage(DownToHalfHealthVoiceLine, "");
		bHalfHealthVoiceLinePlayed = true;
	}
	if (!bLowHealthVoiceLinePlayed &&
		HealthComponent->GetCurrentHealth() - DamageAmount <= HealthComponent->GetMaxHealth() / 10)
	{
		EnqueueVoiceLineWithMessage(LowHealthVoiceLine, "");
		bLowHealthVoiceLinePlayed = true;
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AObjectiveDefendGenerator::SetIsActive(const bool bNewState)
{
	Super::SetIsActive(bNewState);
	if (bNewState)
	{
		GetWorldTimerManager().SetTimer(
			StartWaitTimerHandle,
			this, &AObjectiveDefendGenerator::StartObjective,
			StartDelay,
			false);
	}
}

float AObjectiveDefendGenerator::GetHealthPercentage() const
{
	if (!HealthComponent)
	{
		return 0;
	}
	return HealthComponent->GetCurrentHealth() / HealthComponent->GetMaxHealth();
}

// void AObjectiveDefendGenerator::BindControlPanel()
// {
// 	if (ControlPanel)
// 	{
// 		FPerformDelegate Delegate;
// 		Delegate.AddUObject(this, &AObjectiveDefendGenerator::RegisterControlPanelInteraction);
// 		ControlPanel->SetInteractFunction(Delegate);
// 	}
// }

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
