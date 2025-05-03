// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAI.h"

#include "HealthComponent.h"
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


// Sets default values
AEnemyAI::AEnemyAI()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void AEnemyAI::BeginPlay()
{
	Super::BeginPlay();
	CollisionType = GetCapsuleComponent()->GetCollisionEnabled();
	AIController = Cast<AEnemyAIController>(Controller);
	
	CurrentTarget = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	EnemySpawnManager = GetWorld()->GetSubsystem<UEnemySpawnManagerSubsystem>();

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetMaterial(0, FadeMaterial);
		FadeDMI = MeshComp->CreateDynamicMaterialInstance(0, FadeMaterial);
		if (FadeDMI)
		{
			FadeDMI->SetScalarParameterValue(TEXT("Radial Radius"), 0.0f);
		}
	}
		
	TArray<AObjectiveBase*> AllObjectives = GetWorld()->GetSubsystem<UObjectiveManagerSubsystem>()->GetAllObjectives();
	for (AObjectiveBase* Objective : AllObjectives)
	{
		UE_LOG(LogEngine, Warning, TEXT("Objective found: %s"), *Objective->GetName())
		
		if (Objective && Objective->GetClass()->ImplementsInterface(UAttackable::StaticClass()))
		{
			UE_LOG(LogEngine, Warning, TEXT("Setting up callback functions"))
			//Objective->AddOnObjectiveInProgressFunction(this, &AEnemyAI::AttackObjective);
			Objective->AddOnObjectiveActivatedFunction(this, &AEnemyAI::AttackObjective);
			Objective->AddOnObjectiveDeactivatedFunction(this, &AEnemyAI::AttackPlayer);
		}
	}
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitEnemyStats, 1.f, Context);

		if (Spec.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Init Enemy Spec is valid"));

			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Health"), Health);
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), AttackDamage);
			EnemyAttributeSet = AbilitySystemComponent->GetSet<UEnemyAttributeSet>();
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Init Enemy Spec is NOT valid"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy AbilitySystemComponent is null in BeginPlay!"));
	}
}

void AEnemyAI::Attack()
{
	if (CurrentTarget == nullptr)
	{
		return;
	}
	
	UClass* DamageTypeClass = UDamageType::StaticClass();	
	AController* MyOwnerInstigator = GetOwner()->GetInstigatorController();
	if (EnemyAttributeSet != nullptr)
	{
		AttackDamage = EnemyAttributeSet->Damage.GetBaseValue();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyAttributeSet is null !"));
	}
	const float Damage = EnemyAttributeSet->Damage.GetCurrentValue();
	UE_LOG(LogEngine, Warning, TEXT("Dealing Damage to: %s, %f"), *CurrentTarget.GetObject()->GetName(), Damage)
	UGameplayStatics::ApplyDamage(Cast<AActor>(CurrentTarget.GetObject()), Damage, MyOwnerInstigator, this, DamageTypeClass);
}


float AEnemyAI::GetAttackRange() const
{
	return AttackRange;
}

UHealthComponent* AEnemyAI::GetHealthComponent() const
{
	return HealthComponent;
}

TScriptInterface<IAttackable> AEnemyAI::GetTarget() const
{
	return CurrentTarget;
}

void AEnemyAI::Die()
{
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
	
	Controller->StopMovement();
	Cast<AEnemyAIController>(Controller)->BrainComponent->StopLogic("Dead");
	GetCapsuleComponent()->SetEnableGravity(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DeathStartTime = GetWorld()->GetTimeSeconds();
	bFadeComplete = false;
}

void AEnemyAI::AttackObjective(AObjectiveBase* Objective)
{
	if (!bChangedToTargetPlayer && EnemySpawnManager->GetAliveEnemies().Contains(this))
	{
		UE_LOG(LogEngine, Warning, TEXT("Time to attack objective: %s"), *Objective->GetName())
		CurrentTarget = Objective;
		bChangedToTargetPlayer = true;
	}
}

void AEnemyAI::AttackPlayer(AObjectiveBase* Objective)
{
	UE_LOG(LogEngine, Warning, TEXT("Time to attack the player"))
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
	GetCapsuleComponent()->SetCollisionEnabled(CollisionType);
	GetCapsuleComponent()->SetEnableGravity(true);
	HealthComponent->ResetHealthToMax();
}

void AEnemyAI::OnDeathFXFinished(UNiagaraComponent* PooledNiagaraComp)
{
	UE_LOG(LogEngine, Warning, TEXT("OnDeathFXFinished"));
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
		FVector Location = FVector(10000, 10000, 10000);
		SetActorHiddenInGame(true);
    	SetActorLocation(Location);
    	bChangedToTargetPlayer = false;
    	EnemySpawnManager->MarkEnemyAsDead(this);
		SetActorTickEnabled(false);
}


