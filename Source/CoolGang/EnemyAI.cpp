// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAI.h"

#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Character.h"
#include "EnemySpawnManager.h"

// Sets default values
AEnemyAI::AEnemyAI()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CapsuleComp =  CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Collider"));
	RootComponent = CapsuleComp;
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
	BaseMesh->SetupAttachment(CapsuleComp);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));


	
	
}

void AEnemyAI::BeginPlay()
{
	Super::BeginPlay();


	if (EnemySpawnManagerClass) // Ensure the class variable is set
	{
		// Use the standard function to get the single actor instance
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
}

void AEnemyAI::Attack()
{
	UClass* DamageTypeClass = UDamageType::StaticClass();	
	AController* MyOwnerInstigator = GetOwner()->GetInstigatorController();
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
	if (HealthComponent->GetCurrentHealth() <= 0 && EnemySpawnManager->GetAliveEnemies().Contains(this))
	{
		Die();
	}
}

