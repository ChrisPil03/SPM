// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_PlayerInRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "EnemyAI.h"

UBTService_PlayerInRange::UBTService_PlayerInRange()
{
	NodeName = "Update Player In Range";
}

void UBTService_PlayerInRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	AEnemyAI* AIPawn = Cast<AEnemyAI>(OwnerComp.GetAIOwner()->GetPawn());
	if (AIPawn == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AI Pawn is null"));
		return;
	}
	
	if (PlayerPawn == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Player Pawn is null"));
		return;
	}

	
	if (AIPawn->GetDistanceTo(PlayerPawn) <= AIPawn->GetAttackRange())
	{
		UE_LOG(LogTemp, Warning, TEXT("Distance to Player is: %f and attack range is %f, NPC can attack: %d"), AIPawn->GetDistanceTo(PlayerPawn), AIPawn->GetAttackRange(), AIPawn->GetDistanceTo(PlayerPawn) <= AIPawn->GetAttackRange());
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), false);
	}
}