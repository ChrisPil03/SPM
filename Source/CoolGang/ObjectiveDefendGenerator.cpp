#include "ObjectiveDefendGenerator.h"
#include "HealthComponent.h"
#include "AbilitySystemComponent.h"
#include "DiveGameMode.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GeneratorAttributeSet.h"
#include "InteractableObject.h"
#include "Kismet/GameplayStatics.h"

AObjectiveDefendGenerator::AObjectiveDefendGenerator() :
	MaxHealth(10000.f),
	ActivationDelay(10.f),
	bHalfHealthVoiceLinePlayed(false),
	bLowHealthVoiceLinePlayed(false),
	bIsActivating(false),
	CurrentShield(0.f),
	MaxShield(20000.f)
{
	PrimaryActorTick.bCanEverTick = true;
	SetIsTimeBased(true);
	ScoreType = EScoreType::ObjectiveGeneratorCompleted;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	RootComponent = CapsuleComponent;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(RootComponent);
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void AObjectiveDefendGenerator::BeginPlay()
{
	Super::BeginPlay();

	EnemyTargetSpheres.Empty();
	GetComponents<USphereComponent>(EnemyTargetSpheres);
	
	CurrentShield = MaxShield;
	Health = MaxHealth;
			
			BindControlPanel();
			BindCompletionFunction();
			InitStats();
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UGeneratorAttributeSet::GetHealthAttribute()
			).AddUObject(this, &AObjectiveDefendGenerator::OnCurrentHealthChanged);
		}

void AObjectiveDefendGenerator::OnCurrentHealthChanged(const FOnAttributeChangeData& Data)
{
	Health = Data.NewValue;
	OnGeneratorHealthChangedDelegate.Broadcast(Health);
	
	// if (!bHalfHealthVoiceLinePlayed && Health / MaxHealth <= 0.5f)
	// {
	// 	EnqueueVoiceLine(DownToHalfHealthVoiceLine, -1);
	// 	bHalfHealthVoiceLinePlayed = true;
	// }
	// if (!bLowHealthVoiceLinePlayed && Health / MaxHealth <= 0.1f)
	// {
	// 	EnqueueVoiceLine(LowHealthVoiceLine, -1);
	// 	bLowHealthVoiceLinePlayed = true;
	// }
}

void AObjectiveDefendGenerator::StartObjective()
{
	Super::StartObjective();
}

void AObjectiveDefendGenerator::CompleteObjective()
{
	OnRequestAddScore.Broadcast(ScoreType);

	CurrentShield = MaxShield;
	if (OnShieldChanged.IsBound())
	{
		OnShieldChanged.Broadcast();
	}

	Super::CompleteObjective();
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

void AObjectiveDefendGenerator::ActivateObjective()
{
	ADiveGameMode* GameMode = Cast<ADiveGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode->GameIsOver()) return;
	
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

	if (ControlPanel)
	{
		ControlPanel->ResetInteractable();
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
	if (CurrentShield > 0)
	{
		CurrentShield -= Damage;

		if (CurrentShield <= 0)
		{
			CurrentShield = 0;
			if (OnShieldDestroyed.IsBound())
			{
				OnShieldDestroyed.Broadcast();
			}
			StartActivationProcess();
		}
		if (OnShieldChanged.IsBound())
		{
			OnShieldChanged.Broadcast();
		}
	} else
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_DamageGeneratorHealth, 1.f, Context);

		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Health"), -Damage);  // negative Damage
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void AObjectiveDefendGenerator::StartActivationProcess()
{
	bIsActivating = true;
	EnqueueVoiceLine(ShieldFailingVoiceLine, 0);
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
	if (OnStartActivating.IsBound())
	{
		OnStartActivating.Broadcast();
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

TArray<USphereComponent*> AObjectiveDefendGenerator::GetMovementNodes()
{
	return EnemyTargetSpheres;
}

void AObjectiveDefendGenerator::GetTimeUntilShieldRestored(int32& OutMinutes, int32& OutSeconds) const
{
	int32 ElapsedTime = GetProgressTimer().GetElapsedTime();
	int32 TimeLeft = GetObjectiveTime() - ElapsedTime;
	OutMinutes = TimeLeft / 60;
	OutSeconds = TimeLeft % 60;
}

int32 AObjectiveDefendGenerator::GetSecondsUntilShieldDestroyed() const
{
	if (bIsActivating)
	{
		float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(ActivationDelayTimerHandle);
		return ActivationDelay - static_cast<int32>(ElapsedTime);
	}
	return ActivationDelay;
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
		ControlPanel->InteractDelegate.AddDynamic(this, &AObjectiveDefendGenerator::RegisterControlPanelInteraction);
	}
}

void AObjectiveDefendGenerator::BindCompletionFunction()
{
	FTimerCompletionDelegate Delegate;
	Delegate.BindUObject(this, &AObjectiveDefendGenerator::CompleteObjective);
	GetProgressTimer().SetCompletionDelegate(Delegate);
}
