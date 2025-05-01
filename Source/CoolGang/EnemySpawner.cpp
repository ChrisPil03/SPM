// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "TimerManager.h"
#include "EnemySpawnManagerSubsystem.h"
#include "AbilitySystemComponent.h"
#include "EnemyAI.h"
#include "EnemyAttributeSet.h"
#include "ObjectiveDefendGenerator.h"
#include "ObjectiveManagerSubsystem.h"

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

AEnemyAI* AEnemySpawner::SpawnEnemy() const
{
	UE_LOG(LogTemp, Warning, TEXT("Spawning"))
	FVector Location = GetActorLocation();
	FRotator Rotation = GetActorRotation();

	TArray<AEnemyAI*> DeadEnemies = EnemySpawnManager->GetDeadEnemies();
	if (DeadEnemies.Num() > 0)
	{
		DeadEnemies[0]->SetActorLocationAndRotation(Location, Rotation);
		DeadEnemies[0]->GetHealthComponent()->ResetHealthToMax();
		AEnemyAI* DeadEnemy = Cast<AEnemyAI>(DeadEnemies[0]);
		if (DeadEnemy)
		{
			UAbilitySystemComponent* ASC = DeadEnemy->AbilitySystemComponent;
			if (ASC)
			{
				const UEnemyAttributeSet* AttributeSet = ASC->GetSet<UEnemyAttributeSet>();
				if (AttributeSet)
				{
					float EnemyMaxHealth = AttributeSet->GetMaxHealth();

					// Now set health
					UEnemyAttributeSet* MutableAttributeSet = const_cast<UEnemyAttributeSet*>(AttributeSet);
					MutableAttributeSet->SetHealth(EnemyMaxHealth);
				}
			}
		}
		
		return DeadEnemies[0];
	}

	return GetWorld()->SpawnActor<AEnemyAI>(EnemyClass, Location, Rotation);
}

