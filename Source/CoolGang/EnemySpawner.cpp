// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "TimerManager.h"
#include "EnemySpawnManager.h" 
#include "EnemyAI.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AEnemyAI* AEnemySpawner::SpawnEnemy()
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

