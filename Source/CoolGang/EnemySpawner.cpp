// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "TimerManager.h"

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
	GetWorldTimerManager().SetTimer(
		PeriodicSpawnTimerHandle,
		this,
		&AEnemySpawner::HandlePeriodicSpawn,
		PeriodicSpawnInterval,
		true, // Loop
		PeriodicSpawnInterval // Initial delay same as interval
	);
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemySpawner::HandlePeriodicSpawn()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawning"))
	FVector Location = GetActorLocation();
	FRotator Rotation = GetActorRotation();
	AEnemyAI* Enemy = GetWorld()->SpawnActor<AEnemyAI>(EnemyClass, Location, Rotation);
}

