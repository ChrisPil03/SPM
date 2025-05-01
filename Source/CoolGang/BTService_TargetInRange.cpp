// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TargetInRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "EnemyAI.h"

UBTService_TargetInRange::UBTService_TargetInRange()
{
	NodeName = "Update Player In Range";
}

void UBTService_TargetInRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);


	AEnemyAI* Enemy = Cast<AEnemyAI>(OwnerComp.GetAIOwner()->GetPawn());
	if (Enemy == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No AIPawn"));
		return;
	}
	
	AActor* Target = Cast<AActor>(Enemy->GetTarget().GetObject());
	if (Target == nullptr)
	{
		return;
	}
	UE_LOG(LogEngine, Warning, TEXT("Distance to target: %f\n Attack range is: %f"), Enemy->GetDistanceTo(Target), Enemy->GetAttackRange())
	if (Enemy->GetDistanceTo(Target) <= Enemy->GetAttackRange())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), false);
	}
}

