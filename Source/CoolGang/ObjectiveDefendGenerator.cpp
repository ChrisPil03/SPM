#include "ObjectiveDefendGenerator.h"
#include "HealthComponent.h"
#include "ScoreManagerComponent.h"
#include "AbilitySystemComponent.h"
#include "DiveGameMode.h"
#include "Components/CapsuleComponent.h"
#include "GeneratorAttributeSet.h"
#include "InteractableObject.h"
#include "Kismet/GameplayStatics.h"

AObjectiveDefendGenerator::AObjectiveDefendGenerator() :
	ActivationDelay(10.f),
	bHalfHealthVoiceLinePlayed(false),
	bLowHealthVoiceLinePlayed(false),
	bIsActivating(false),
	CurrentShield(0.f),
	MaxShield(20000.f)
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
	BindCompletionFunction();
	InitStats();
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UGeneratorAttributeSet::GetHealthAttribute()
	).AddUObject(this, &AObjectiveDefendGenerator::OnCurrentHealthChanged);

	CurrentShield = MaxShield;
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
	CurrentShield = MaxShield;
	if (OnShieldChanged.IsBound())
	{
		OnShieldChanged.Broadcast();
	}
}

void AObjectiveDefendGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetIsInProgress())
	{
		IncreaseObjectiveProgress(DeltaSeconds);

		if (OnUniqueProgressChanged.IsBound())
		{
			OnUniqueProgressChanged.Broadcast();
		}
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

void AObjectiveDefendGenerator::ActivateObjective()
{
	bIsActivating = false;
	if (ObjectiveManager)
	{
		ObjectiveManager->DeactivateAllSubObjectives();
		ObjectiveManager->ActivateMainObjective();	
	}
	ResetProgress();
	SetObjectiveState(EObjectiveState::NotStarted);
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

	if (OnUniqueProgressChanged.IsBound())
	{
		OnUniqueProgressChanged.Broadcast();
	}
	
	// if (GE_ResetGeneratorHealth)
	// {
	// 	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	// 	AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(GE_ResetGeneratorHealth, 1.f, Context);
	// }
}

void AObjectiveDefendGenerator::SetIsActive(const bool bNewState)
{
	Super::SetIsActive(bNewState);
	if (ControlPanel)
	{
		ControlPanel->SetCanInteractWith(bNewState);
	}
}

FVector AObjectiveDefendGenerator::GetWaypointTargetLocation() const
{
	if (ControlPanel)
	{
		// FVector Origin;
		// FVector Extent;
		// ControlPanel->GetActorBounds(false, Origin, Extent, false);
		return ControlPanel->GetActorLocation();
	}
	return Super::GetWaypointTargetLocation();
}

void AObjectiveDefendGenerator::FailObjective()
{
	if (!GetIsFailed())
	{
		SetObjectiveState(EObjectiveState::Failed);
		SetIsActive(false);
		Cast<ADiveGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->EndGame();	
	}
}

void AObjectiveDefendGenerator::DamageGeneratorShield(const float Damage)
{
	if (!GetIsActive() && CurrentShield > 0)
	{
		CurrentShield -= Damage;

		if (CurrentShield <= 0)
		{
			CurrentShield = 0;
			bIsActivating = true;
			GetWorld()->GetTimerManager().SetTimer(
				ActivationDelayTimerHandle,
				this,
				&AObjectiveDefendGenerator::ActivateObjective,
				ActivationDelay,
				false);
			if (ObjectiveManager)
			{
				ObjectiveManager->DeactivateAllSubObjectives();
			}
		}

		if (OnShieldChanged.IsBound())
		{
			OnShieldChanged.Broadcast();
		}
	}
}

TArray<FString> AObjectiveDefendGenerator::GetUniqueObjectiveProgress() const
{
	int32 OutMinutes;
	int32 OutSeconds;
	GetTimeUntilShieldRestored(OutMinutes, OutSeconds);
	FString Minutes;
	FString Seconds;
	if (OutMinutes <= 9)
	{
		Minutes = FString::Printf(TEXT("0%d"), OutMinutes);
	}else
	{
		Minutes = FString::Printf(TEXT("%d"), OutMinutes);
	}
	if (OutSeconds <= 9)
	{
		Seconds = FString::Printf(TEXT("0%d"), OutSeconds);
	}else
	{
		Seconds = FString::Printf(TEXT("%d"), OutSeconds);
	}
	
	return {
		FString::Printf(TEXT("Time until Shield restored: %s:%s"), *Minutes, *Seconds)
	};
}

void AObjectiveDefendGenerator::GetTimeUntilShieldRestored(int32& OutMinutes, int32& OutSeconds) const
{
	int32 ElapsedTime = GetProgressTimer().GetElapsedTime();
	int32 TimeLeft = GetObjectiveTime() - ElapsedTime;
	OutMinutes = TimeLeft / 60;
	OutSeconds = TimeLeft % 60;
}

float AObjectiveDefendGenerator::GetHealthPercentage() const
{
	const UGeneratorAttributeSet* MyAttributes = AbilitySystemComponent->GetSet<UGeneratorAttributeSet>();
	
	return MyAttributes->GetHealth() / MyAttributes->GetMaxHealth();
}

float AObjectiveDefendGenerator::GetShieldPercentage() const
{
	return CurrentShield / MaxShield;
}

void AObjectiveDefendGenerator::BindControlPanel()
{
	if (ControlPanel)
	{
		ControlPanel->SetOnInteractFunction(this, &AObjectiveDefendGenerator::RegisterControlPanelInteraction);
	}
}

void AObjectiveDefendGenerator::BindCompletionFunction()
{
	FTimerCompletionDelegate Delegate;
	Delegate.BindUObject(this, &AObjectiveDefendGenerator::CompleteObjective);
	GetProgressTimer().SetCompletionDelegate(Delegate);
}
