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
		UE_LOG(LogTemp, Warning, TEXT("UBTService_Target: OwnerController is null."));
		return;
	}

	APawn* OwnerPawn = OwnerController->GetPawn();
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTService_Target: OwnerPawn is null."));
		return;
	}

	AEnemyAI* EnemyAI = Cast<AEnemyAI>(OwnerPawn);
	if (!EnemyAI)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTService_Target: Pawn is not an AEnemyAI. Pawn is: %s"), *GetNameSafe(OwnerPawn));
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
