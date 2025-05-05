// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "TimerManager.h"
#include "EnemySpawnManagerSubsystem.h"
#include "AbilitySystemComponent.h"
#include "EnemyAI.h"
#include "EnemyAttributeSet.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	EnemySpawnManager = GetWorld()->GetSubsystem<UEnemySpawnManagerSubsystem>();
	EnemySpawnManager->RegisterSpawner(SpawnArea, this);
}


void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AEnemyAI* AEnemySpawner::ReuseDeadEnemy(AEnemyAI* Enemy) const
{
	if (Enemy)
	{
		UAbilitySystemComponent* ASC = Enemy->AbilitySystemComponent;
		if (ASC)
		{
			const UEnemyAttributeSet* AttributeSet = ASC->GetSet<UEnemyAttributeSet>();
			if (AttributeSet)
			{
				float EnemyMaxHealth = AttributeSet->GetMaxHealth();
				
				UEnemyAttributeSet* MutableAttributeSet = const_cast<UEnemyAttributeSet*>(AttributeSet);
				MutableAttributeSet->SetHealth(EnemyMaxHealth);
			}
		}
		SetEnemyPosition(Enemy);
		Enemy->SetAlive();
	}
	return Enemy;
}

void AEnemySpawner::SetEnemyPosition(AEnemyAI* Enemy) const
{
	if (!IsValid(Enemy))
	{
		return;
	}
	FVector Location = GetActorLocation();
	FRotator Rotation = GetActorRotation();
	Enemy->SetActorLocationAndRotation(Location, Rotation);
}

AEnemyAI* AEnemySpawner::SpawnEnemy() const
{
	FVector Location = GetActorLocation();
	FRotator Rotation = GetActorRotation();
	
	int32 RandomIndex = FMath::RandRange(0, EnemyClassArray.Num() - 1);
	TArray<AEnemyAI*> DeadEnemies = EnemySpawnManager->GetDeadEnemies();
	if (DeadEnemies.Num() > 0)
	{
		return ReuseDeadEnemy(DeadEnemies[0]);
	}
	return GetWorld()->SpawnActor<AEnemyAI>(EnemyClassArray[RandomIndex], Location, Rotation);
}

void AEnemySpawner::RelocateEnemy(AEnemyAI* Enemy) const
{
	SetEnemyPosition(Enemy);
}


