// Fill out your copyright notice in the Description page of Project Settings.

#include "DiveGameMode.h"
#include <cmath>
#include <algorithm>

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
}

void ADiveGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bGameHasEnded)
	{
		HandleElapsedTime(DeltaSeconds);
		NextObjectiveTimer -= DeltaSeconds;
		if (NextObjectiveTimer <= 0)
		{
			NextObjectiveTimer = ComputeTimer(ObjectiveCount++, BaselineObjectiveTimer, MinimumObjectiveTimer,TimeScalingValue);
			//UE_LOG(LogTemp, Warning, TEXT("Activating random objective"))
			GetWorld()->GetSubsystem<UObjectiveManagerSubsystem>()->ActivateRandomObjective(ObjectiveMalfunctionTimer, 0.1, 10);
	
		}
	}
}

void ADiveGameMode::EndGame()
{
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


float ADiveGameMode::ComputeTimer(int cycleIndex, float T0, float Tmin, float k)
{
	float t = T0 - k * std::logf(cycleIndex + 1);
	return std::max(Tmin, t);
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
