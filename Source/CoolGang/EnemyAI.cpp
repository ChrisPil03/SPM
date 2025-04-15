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

// Called when the game starts or when spawned
void AEnemyAI::BeginPlay()
{
	Super::BeginPlay();
	
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
	if (HealthComponent->GetCurrentHealth() <= 0)
	{
		Die();
	}
}

