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
		// UE_LOG(LogTemp, Warning, TEXT("Enemy is nullptr"));
		return;
	}
	
	AActor* Target = Cast<AActor>(Enemy->GetTarget().GetObject());
	if (Target == nullptr)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Target is nullptr"));
		return;
	}

	if (Enemy->GetSquaredDistanceTo(Target) <= FMath::Pow(Enemy->GetAttackRange(), 2) )
	{
		Enemy->SetTargetInRange(true);
	}
	else
	{
		Enemy->SetTargetInRange(false);
	}
	
	OwnerComp.GetBlackboardComponent()->SetValueAsFloat(GetSelectedBlackboardKey(), Enemy->GetSquaredDistanceTo(Target));
}

