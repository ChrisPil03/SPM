// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"

#include "AIController.h"
#include "EnemyAI.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	AEnemyAI* EnemyAI = Cast<AEnemyAI>(OwnerComp.GetAIOwner()->GetPawn());
	if (EnemyAI == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	UE_LOG(LogTemp, Warning, TEXT("Enemy %s is attacking"), *EnemyAI->GetClass()->GetName())
	EnemyAI->Attack();
	
	return EBTNodeResult::Succeeded;
}

