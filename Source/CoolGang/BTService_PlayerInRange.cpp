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
		UE_LOG(LogTemp, Warning, TEXT("No AIPawn"));
		return;
	}
	
	if (PlayerPawn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Player"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Distance to player is %f and attack range is %f"), AIPawn->GetDistanceTo(PlayerPawn), AIPawn->GetAttackRange());
	if (AIPawn->GetDistanceTo(PlayerPawn) <= AIPawn->GetAttackRange())
	{
		
		
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), false);
	}
}

