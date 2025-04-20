// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_JumpThreshold.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyAI.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_JumpThreshold::UBTService_JumpThreshold()
{
	NodeName = "Update Jump Threshold";
}

void UBTService_JumpThreshold::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	AEnemyAI* AIPawn = Cast<AEnemyAI>(OwnerComp.GetAIOwner()->GetPawn());
	if (AIPawn == nullptr)
	{
		return;
	}
	
	if (PlayerPawn == nullptr)
	{
		return;
	}

	float CurrentJumpThreshold = FMath::RandRange(AIPawn->GetAttackRange(), JumpThreshold);

	UE_LOG(LogTemp, Warning, TEXT("Distance to Player: %f \n Attack Range: %f \n Current Jump Threshold: %f"), AIPawn->GetDistanceTo(PlayerPawn), AIPawn->GetAttackRange(), CurrentJumpThreshold);
	if (AIPawn->GetDistanceTo(PlayerPawn) >= AIPawn->GetAttackRange() && AIPawn->GetDistanceTo(PlayerPawn) <= CurrentJumpThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("Jumping"));
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), false);
	}

}