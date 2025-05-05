#include "BTService_TargetInLineOfSight.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyAI.h"

UBTService_TargetInLineOfSight::UBTService_TargetInLineOfSight()
{
	NodeName = "Update If Line Of Sight To Player";
}

void UBTService_TargetInLineOfSight::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return;
	}
	AEnemyAI* Enemy = Cast<AEnemyAI>(OwnerComp.GetAIOwner()->GetPawn());

	if (Enemy == nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("No AIPawn"));
		return;
	}
	
	AActor* Target = Cast<AActor>(Enemy->GetTarget().GetObject());

	if (Target == nullptr)
	{
		return;
	}
	
	if (OwnerComp.GetAIOwner()->LineOfSightTo(Target))
	{

		//UE_LOG(LogEngine, Warning, TEXT("Can see the target"));
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}
	else
	{
		//UE_LOG(LogEngine, Warning, TEXT("Cannot see the target"));
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), false);
	}
}
