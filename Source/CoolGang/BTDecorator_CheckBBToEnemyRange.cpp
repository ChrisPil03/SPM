// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_CheckBBToEnemyRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "EnemyAI.h"

UBTDecorator_CheckBBToEnemyRange::UBTDecorator_CheckBBToEnemyRange()
{
	NodeName = "Is Within Range";
}

bool UBTDecorator_CheckBBToEnemyRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* Controller = OwnerComp.GetAIOwner();

	if (!BlackboardComp || !Controller)
	{
		return false;
	}

	AEnemyAI* Pawn = Cast<AEnemyAI>(Controller->GetPawn());
	if (!Pawn)
	{
		return false;
	}

	float DistanceSq = BlackboardComp->GetValueAsFloat(GetSelectedBlackboardKey());

	float AttackRange = Pawn->GetAttackRange();

	float AttackRangeSq = AttackRange * AttackRange;

	return DistanceSq <= AttackRangeSq;
}

FString UBTDecorator_CheckBBToEnemyRange::GetStaticDescription() const
{
	FName KeyName = BlackboardKey.SelectedKeyName;
	
	 return FString::Printf(TEXT("%s: Is '%s' <= (Enemy Attack Range)^2"),
	 *Super::GetStaticDescription(),
	 *KeyName.ToString());
}
