#include "ObjectiveDefendGenerator.h"
#include "HealthComponent.h"
#include "ScoreManagerComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GeneratorAttributeSet.h"
#include "InteractableObject.h"

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

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
}

void AObjectiveDefendGenerator::BeginPlay()
{
	Super::BeginPlay();
	BindControlPanel();
	BindDeathFunction();
	BindCompletionFunction();
	InitStats();
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UGeneratorAttributeSet::GetHealthAttribute()
	).AddUObject(this, &AObjectiveDefendGenerator::OnCurrentHealthChanged);
	
}

void AObjectiveDefendGenerator::OnCurrentHealthChanged(const FOnAttributeChangeData& Data)
{
	float NewCurrentHealth = Data.NewValue;
	OnGeneratorHealthChangedDelegate.Broadcast(NewCurrentHealth);
	
	if (!bHalfHealthVoiceLinePlayed && (NewCurrentHealth/ MaxHealth <= 0.5f))
	{
		EnqueueVoiceLineWithMessage(DownToHalfHealthVoiceLine, "");
		bHalfHealthVoiceLinePlayed = true;
	}
	if (!bLowHealthVoiceLinePlayed &&
		HealthComponent->GetCurrentHealth() - (NewCurrentHealth/ MaxHealth <= 0.1f))
	{
		EnqueueVoiceLineWithMessage(LowHealthVoiceLine, "");
		bLowHealthVoiceLinePlayed = true;
	}
}

void AObjectiveDefendGenerator::StartObjective()
{
	Super::StartObjective();
}

void AObjectiveDefendGenerator::CompleteObjective()
{
	Super::CompleteObjective();
	OnRequestAddScore.Broadcast(EScoreType::ObjectiveGeneratorCompleted);
}

void AObjectiveDefendGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaSeconds);
	}
}

void AObjectiveDefendGenerator::RegisterControlPanelInteraction(AInteractableObject* InteractableObject)
{
	if (!GetIsActive())
	{
		return;
	}
	StartObjective();
}

bool AObjectiveDefendGenerator::CannotTakeDamage() const
{
	return !GetIsActive() || GetIsComplete() || GetIsFailed() || GetIsNotStarted();
}

void AObjectiveDefendGenerator::InitStats()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitGeneratorStats, 1.f, Context);

		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Health"), Health);
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			
		}
	}	
}

void AObjectiveDefendGenerator::ResetObjective()
{
	Super::ResetObjective();
	bHalfHealthVoiceLinePlayed = false;
	bLowHealthVoiceLinePlayed = false;
	
	if (GE_ResetGeneratorHealth)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(GE_ResetGeneratorHealth, 1.f, Context);
	}
}

// float AObjectiveDefendGenerator::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
// 	AController* EventInstigator, AActor* DamageCauser)
// {
// 	if (CannotTakeDamage())
// 	{
// 		//UE_LOG(LogEngine, Warning, TEXT("Cannot take damage"));
// 		return 0;
// 	}
// 	//UE_LOG(LogEngine, Warning, TEXT("Taking %f damage!"), DamageAmount);
// 	
// 	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
// }

void AObjectiveDefendGenerator::SetIsActive(const bool bNewState)
{
	Super::SetIsActive(bNewState);
	ControlPanel->SetCanInteractWith(bNewState);
}

float AObjectiveDefendGenerator::GetHealthPercentage() const
{
	const UGeneratorAttributeSet* MyAttributes = AbilitySystemComponent->GetSet<UGeneratorAttributeSet>();
	
	return MyAttributes->GetHealth() / MyAttributes->GetMaxHealth();
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
