#include "BTService_Target.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "EnemyAI.h"

UBTService_Target::UBTService_Target()
{
	NodeName = "Update Target";
}

void UBTService_Target::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* OwnerController = OwnerComp.GetAIOwner();
	if (!OwnerController)
	{
		return;
	}

	APawn* OwnerPawn = OwnerController->GetPawn();
	if (!OwnerPawn)
	{
		return;
	}

	AEnemyAI* EnemyAI = Cast<AEnemyAI>(OwnerPawn);
	if (!EnemyAI)
	{
		return;
	}

	TScriptInterface<IAttackable> TargetInterface = EnemyAI->GetTarget();
	UObject* TargetObject = TargetInterface.GetObject();
	
	if (TargetObject == nullptr)
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
		return;
	}

	
	OwnerComp.GetBlackboardComponent()->SetValueAsObject(GetSelectedBlackboardKey(), TargetObject);
}
