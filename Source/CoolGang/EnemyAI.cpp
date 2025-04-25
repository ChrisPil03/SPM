// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAI.h"

#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Character.h"
#include "EnemySpawnManager.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"

#include "EnemyAttributeSet.h"

// Sets default values
AEnemyAI::AEnemyAI()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	
	
}

void AEnemyAI::BeginPlay()
{
	Super::BeginPlay();


	if (EnemySpawnManagerClass) // Ensure the class variable is set
	{
		AEnemySpawnManager* FoundManager = Cast<AEnemySpawnManager>(UGameplayStatics::GetActorOfClass(GetWorld(), EnemySpawnManagerClass));

		if (IsValid(FoundManager)) // Use IsValid() to check for null and pending kill
		{
			EnemySpawnManager = FoundManager; // Assign if found
			UE_LOG(LogTemp, Display, TEXT("AI %s found EnemySpawnManager: %s"), *GetName(), *EnemySpawnManager->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AI %s could NOT find any actor of class %s!"), *GetName(), *EnemySpawnManagerClass->GetName());
			// EnemySpawnManager remains null or whatever its default was
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AI %s has EnemySpawnManagerClass unset! Cannot search."), *GetName());
	}

	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitEnemyStats, 1.f, Context);

		if (Spec.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Init Enemy Spec is valid"));

			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Health"), 1);
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), 100);
			EnemyAttributeSet = AbilitySystemComponent->GetSet<UEnemyAttributeSet>();
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Init Spec is NOT valid"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AbilitySystemComponent is null in BeginPlay!"));
	}
}
	


void AEnemyAI::Attack()
{
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
	
	UGameplayStatics::ApplyDamage(Cast<AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)), AttackDamage, MyOwnerInstigator, this, DamageTypeClass);
}


float AEnemyAI::GetAttackRange() const
{
	return AttackRange;
}

UHealthComponent* AEnemyAI::GetHealthComponent() const
{
	return HealthComponent;
}

void AEnemyAI::Die()
{
	FVector Location = FVector(10000, 10000, 10000);
	
	SetActorLocation(Location);
	EnemySpawnManager->MarkEnemyAsDead(this);
}

void AEnemyAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (EnemySpawnManager == nullptr)
	{
		return;
	}
	
	if (HealthComponent->GetCurrentHealth() <= 0 && EnemySpawnManager->GetAliveEnemies().Contains(this))
	{
		
		Die();

		
	}
}

