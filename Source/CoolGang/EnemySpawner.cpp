// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "TimerManager.h"
#include "EnemySpawnManager.h" 
#include "EnemyAI.h"
#include "GameFramework/FloatingPawnMovement.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

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
		return DeadEnemies[0];
	}
	return GetWorld()->SpawnActor<AEnemyAI>(EnemyClass, Location, Rotation);
}

