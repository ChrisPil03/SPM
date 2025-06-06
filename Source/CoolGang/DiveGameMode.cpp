// Fill out your copyright notice in the Description page of Project Settings.

#include "DiveGameMode.h"
#include <cmath>
#include <algorithm>

#include "EnemySpawnManagerSubsystem.h"
#include "ObjectiveManagerSubsystem.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

void ADiveGameMode::BeginPlay()
{
	Super::BeginPlay();
}


void ADiveGameMode::PlayerKilled(APlayerCharacter* PlayerKilled)
{
	Super::PlayerKilled(PlayerKilled);
	APlayerController* PlayerController = Cast<APlayerController>(PlayerKilled->GetController());
	if (PlayerController != nullptr)
	{
		EndGame();
	}
}

float ADiveGameMode::GetTimeUntilNextObjective() const
{
	return NextObjectiveTimer;
}

ADiveGameMode::ADiveGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	UpdatedSpawnInterval = BaselineSpawnInterval;
	SpawnInterval = BaselineSpawnInterval;
	SpawnIntervalIncreaseCount = 0;
	SpawnIntervalIncreaseProgress = SpawnIntervalIncreaseTimer;
}

void ADiveGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bGameIsActive && !bGameHasEnded)
	{
		HandleElapsedTime(DeltaSeconds);
		NextObjectiveTimer -= DeltaSeconds;
		if (NextObjectiveTimer <= 0)
		{
			NextObjectiveTimer = CalculateObjectiveTimer(ObjectiveCount++, BaselineObjectiveTimer, MinimumObjectiveTimer,TimeScalingValue);
			//UE_LOG(LogTemp, Warning, TEXT("Activating random objective"))
			GetWorld()->GetSubsystem<UObjectiveManagerSubsystem>()->ActivateRandomObjective(ObjectiveMalfunctionTimer, 0.1, 10);
	
		}

		SpawnInterval -= DeltaSeconds;
		if (SpawnInterval <= 0.f)
		{
			GetWorld()->GetSubsystem<UEnemySpawnManagerSubsystem>()->SpawnEnemy();
			SpawnInterval = UpdatedSpawnInterval;
		}

		SpawnIntervalIncreaseProgress -= DeltaSeconds;
		if (SpawnIntervalIncreaseProgress <= 0.f)
		{
			UpdatedSpawnInterval = CalculateSpawnTimer(SpawnIntervalIncreaseCount++, BaselineSpawnInterval, MinimumSpawnInterval, SpawnIntervalScale, MaxSpawnIntervalIncreaseCount, SpawnAccelerationRate);

			SpawnIntervalIncreaseProgress = SpawnIntervalIncreaseTimer;
		}
	}
}

void ADiveGameMode::EndGame()
{
	if (OnGameOver.IsBound())
	{
		OnGameOver.Broadcast();
	}
	UE_LOG(LogTemp, Display, TEXT("EndGame"));
	bGameIsActive = false;
	bGameHasEnded = true;
	AController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->GameHasEnded(Controller->GetPawn(), false);

	//Save the player data
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerCharacter)
	{
		// Call a method or trigger an event on the player character
		PlayerCharacter->SaveScore();
	}
}

void ADiveGameMode::GetElapsedMinutesAndSeconds(int32& OutMinutes, int32& OutSeconds) const
{
	OutMinutes = static_cast<int32>(ElapsedTime) / 60;
	OutSeconds = static_cast<int32>(ElapsedTime) % 60;
}


float ADiveGameMode::CalculateObjectiveTimer(int cycleIndex, float T0, float Tmin, float k)
{
	float t = T0 - k * std::logf(cycleIndex + 1);
	return std::max(Tmin, t);
}

float ADiveGameMode::CalculateSpawnTimer(int cycleIndex, float baselineInterval, float minimumInterval, float intervalScale, int maxCycles, float exponent)
{

	float tNorm = (maxCycles <= 0) ? 0.0f : FMath::Clamp(float(cycleIndex) / float(maxCycles), 0.0f, 1.0f);

	float deltaNorm = FMath::Pow(tNorm, exponent);

	float t = baselineInterval
			- intervalScale * deltaNorm;

	return FMath::Max(minimumInterval, t);
}

void ADiveGameMode::HandleElapsedTime(const float DeltaTime)
{
	int32 PreviousMinute = FMath::FloorToInt32(ElapsedTime / 60.f);
	ElapsedTime += DeltaTime;
	int32 CurrentMinute = FMath::FloorToInt32(ElapsedTime / 60.f);

	if (CurrentMinute > PreviousMinute && OnMinutePassed.IsBound())
	{
		OnMinutePassed.Broadcast();
	}
}
