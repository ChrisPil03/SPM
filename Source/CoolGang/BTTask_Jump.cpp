// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Jump.h"
#include "AIController.h"
#include "EnemyAI.h"

UBTTask_Jump::UBTTask_Jump()
{
	NodeName = TEXT("Jump");
}

EBTNodeResult::Type UBTTask_Jump::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	EnemyAI->Jump();
	
	return EBTNodeResult::Succeeded;
}