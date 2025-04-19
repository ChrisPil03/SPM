// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "PlayerCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	
	if (AIBehavior != nullptr)
	{
		RunBehaviorTree(AIBehavior);

		APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (PlayerCharacter == nullptr)
		{
			return;
		}
		
		if (!PlayerCharacter->IsDead())
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("PlayerLocation"), PlayerCharacter->GetActorLocation());
		}
	}
}

bool AEnemyAIController::IsDead() const
{
	APlayerCharacter* ControlledCharacter = Cast<APlayerCharacter>(GetPawn());

	if (ControlledCharacter != nullptr)
	{
		return ControlledCharacter->IsDead();
	}
	
	return true;
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
