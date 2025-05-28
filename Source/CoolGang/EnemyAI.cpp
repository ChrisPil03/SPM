// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAI.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Character.h"
#include "EnemySpawnManagerSubsystem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "ObjectiveManagerSubsystem.h"
#include "EnemyAttributeSet.h"
#include "ObjectiveBase.h"
#include "Attackable.h"
#include "BrainComponent.h"
#include "EnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "EnemyAIController.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "ObjectiveDefendGenerator.h"
#include "ScoreManagerComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PawnMovementComponent.h"


// Sets default values
AEnemyAI::AEnemyAI()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio Component"));
	AudioComponent->SetupAttachment(RootComponent);
}

void AEnemyAI::BeginPlay()
{
	Super::BeginPlay();
	
	AIController = Cast<AEnemyAIController>(Controller);
	AIController->RunBehaviorTree(BehaviorTree);
	CurrentTarget = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	EnemySpawnManager = GetWorld()->GetSubsystem<UEnemySpawnManagerSubsystem>();

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		FadeDMI = MeshComp->CreateDynamicMaterialInstance(0);
		if (FadeDMI)
		{
			FadeDMI->SetScalarParameterValue(TEXT("Radial Radius"), 0.0f);
		}
	}
		
	// TArray<AObjectiveBase*> AllObjectives = GetWorld()->GetSubsystem<UObjectiveManagerSubsystem>()->GetAllSubObjectives();
	// for (AObjectiveBase* Objective : AllObjectives)
	// {
	// 	if (Objective && Objective->GetClass()->ImplementsInterface(UAttackable::StaticClass()))
	// 	{
	// 		//Objective->AddOnObjectiveInProgressFunction(this, &AEnemyAI::AttackObjective);
	// 		Objective->AddOnObjectiveActivatedFunction(this, &AEnemyAI::AttackObjective);
	// 		Objective->AddOnObjectiveDeactivatedFunction(this, &AEnemyAI::AttackPlayer);
	// 	}
	// }
	if (AObjectiveDefendGenerator* MainObjective = GetWorld()->GetSubsystem<UObjectiveManagerSubsystem>()->GetMainObjective())
	{
		MainObjective->AddOnObjectiveActivatedFunction(this, &AEnemyAI::AttackObjective);
		MainObjective->AddOnObjectiveDeactivatedFunction(this, &AEnemyAI::AttackPlayer);
	}
	
	
	if (MovementSound)
	{
		AudioComponent->SetSound(MovementSound);
		AudioComponent->Play();
	}

	
	GiveAbilities();
	InitEnemyStats();
}

void AEnemyAI::StartDeathSequence()
{
	if (bIsDead)
	{
		return;
	}
	
	bIsDead = true;

	PerformPreDeathActions();
	Die();
}

void AEnemyAI::InitEnemyStats()
{
	if (AbilitySystemComponent)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitEnemyStats, 1.f, Context);

		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Health"), Health);
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), AttackDamage);
			EnemyAttributeSet = AbilitySystemComponent->GetSet<UEnemyAttributeSet>();
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			
		}
	}
}

void AEnemyAI::GiveAbilities()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (AbilitySystemComponent && AttackAbilityClass)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbilityClass, 1, 0, this));
	}
}


void AEnemyAI::Attack()
{
	bIsAttacking = true;
	if (CurrentTarget == nullptr)
	{
		return;
	}

	if (EnemyAttributeSet != nullptr)
	{
		AttackDamage = EnemyAttributeSet->Damage.GetBaseValue();
	}
	
	// if (DamagedActor->ActorHasTag("Player") && IsPlayerShieldActive(DamagedActor))
	// {
	// 	//Reduce damage here ???
	// }
	AbilitySystemComponent->TryActivateAbilityByClass(AttackAbilityClass);
}

bool AEnemyAI::IsPlayerShieldActive(AActor* PlayerActor)
{
	if (PlayerActor)
	{
		UAbilitySystemComponent* PlayerASC = PlayerActor->FindComponentByClass<UAbilitySystemComponent>();
		if (PlayerASC)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found PlayerActor"));
			FGameplayTag ShieldTag = FGameplayTag::RequestGameplayTag(TEXT("State.ShieldActive"));
			return PlayerASC->HasMatchingGameplayTag(ShieldTag);
		}
	}
	return false;
}

float AEnemyAI::GetAttackRange() const
{
	return AttackRange;
}


TScriptInterface<IAttackable> AEnemyAI::GetTarget() const
{
	return CurrentTarget;
}

void AEnemyAI::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy dying"))

	SetActorEnableCollision(false);

	DropUpgrade();
	UNiagaraComponent* NiComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	  GetWorld(),
	  DeathVFX,
	  GetActorLocation(),
	  GetActorRotation(),
	  FVector(1.f,1.f,1.f),
	  true,
	  true,
	  ENCPoolMethod::None,
	  false
	);
	
	if (NiComp)
	{
		NiComp->OnSystemFinished.AddDynamic(this, &AEnemyAI::OnDeathFXFinished);
	}
	if (Controller)
	{
		Controller->StopMovement();
		
		Cast<AEnemyAIController>(Controller)->BrainComponent->StopLogic("Dead");
		
		UBlackboardComponent* Blackboard = Cast<AEnemyAIController>(Controller)->BrainComponent->GetBlackboardComponent();
		if (!Blackboard)
		{
			return;
		}
		
		UBlackboardData* BlackboardData = Blackboard->GetBlackboardAsset();
		if (!BlackboardData)
		{
			return;
		}

		for (auto BlackboardValue : BlackboardData->Keys)
		{
			Blackboard->ClearValue(BlackboardValue.EntryName);
		}
		GetMovementComponent()->Velocity.Set(0.f,0.f,0.f);
		Cast<AEnemyAIController>(Controller)->BrainComponent->Cleanup();
		Cast<AEnemyAIController>(Controller)->BrainComponent->GetBlackboardComponent()->SetValueAsFloat("DistanceToTargetSquared", 100000000000000.f);
	}

	AudioComponent->FadeOut(0.5f, 0.0f);
	
	DeathStartTime = GetWorld()->GetTimeSeconds();
	bFadeComplete = false;
	GiveScore();
}

void AEnemyAI::DropUpgrade()
{
	
	float RandomValue = FMath::FRand(); // Range: 0.0 to 1.0
	if (RandomValue <= DropRate)
	{
		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = GetActorRotation();
		UE_LOG(LogTemp, Warning, TEXT("Drop"));
		GetWorld()->SpawnActor<AActor>(Drop, SpawnLocation, SpawnRotation);
	}
	
}

AActor* AEnemyAI::GetCurrentTarget() const
{
	return  Cast<AActor>(CurrentTarget.GetObject());
}

void AEnemyAI::AttackObjective(AObjectiveBase* Objective)
{
	if (EnemyType == EEnemyType::Gloorb)
	{
		return;
	}
	const TArray<AEnemyAI*> AliveEnemies = EnemySpawnManager->GetAliveEnemiesMap().Find(GetClass())->Enemies;
	if (AliveEnemies.Num() == 0) return;
	
	if (!bChangedToTargetPlayer && AliveEnemies.Contains(this))
	{
		CurrentTarget = Objective;
		bChangedToTargetPlayer = true;
	}
}

void AEnemyAI::GiveScore()
{
	switch (EnemyType) {
	case EEnemyType::Spider: OnRequestAddScore.Broadcast(EScoreType::SpiderKill); break;
	case EEnemyType::Wasp: OnRequestAddScore.Broadcast(EScoreType::WaspKill); break;
	default: break;
	}
}

void AEnemyAI::AttackPlayer(AObjectiveBase* Objective)
{
	CurrentTarget = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

void AEnemyAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (EnemySpawnManager == nullptr)
	{
		return;
	}
	
	if (bFadeComplete)
	{
		return;
	}
	
	float Elapsed = GetWorld()->GetTimeSeconds() - DeathStartTime;
	float Alpha   = FMath::Clamp(Elapsed / FadeDuration, 0.f, 1.f);

	FadeDMI->SetScalarParameterValue(TEXT("Radial Radius"), Alpha);

	if (Alpha >= 1.f)
	{
		bFadeComplete = true;
		OnFadeFinished();
	}
}

void AEnemyAI::SetAlive()
{
	SetActorTickEnabled(true);
	if (FadeDMI)
	{
		FadeDMI->SetScalarParameterValue(TEXT("Radial Radius"), 0.0f);
	}
	SetActorHiddenInGame(false);
	AIController->RunBehaviorTree(BehaviorTree);
	SetActorEnableCollision(true);
	//Should call this in BP
	if (GE_ResetHealth)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(GE_ResetHealth, 1.f, Context);
	}
	AudioComponent->Play();
	bIsDead = false;
}

void AEnemyAI::OnDeathFXFinished(UNiagaraComponent* PooledNiagaraComp)
{
	bDeathVFXComplete = true;
	if (bFadeComplete)
	{
		ReleaseToPool();
	}
}

void AEnemyAI::OnFadeFinished()
{
	if (bDeathVFXComplete)
	{
		ReleaseToPool();
	}
}

void AEnemyAI::ReleaseToPool()
{
		SetActorHiddenInGame(true);
    	bChangedToTargetPlayer = false;
    	EnemySpawnManager->MarkEnemyAsDead(this);
		SetActorTickEnabled(false);
}

