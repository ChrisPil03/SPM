#include "BTService_TargetLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "EnemyAI.h"

UBTService_TargetLocation::UBTService_TargetLocation()
{
	NodeName = "Update Target Location";
}

void UBTService_TargetLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* OwnerController = OwnerComp.GetAIOwner();
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTService_TargetLocation: OwnerController is null."));
		return;
	}

	APawn* OwnerPawn = OwnerController->GetPawn();
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTService_TargetLocation: OwnerPawn is null."));
		return;
	}

	AEnemyAI* EnemyAI = Cast<AEnemyAI>(OwnerPawn);
	if (!EnemyAI)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTService_TargetLocation: Pawn is not an AEnemyAI. Pawn is: %s"), *GetNameSafe(OwnerPawn));
		return;
	}

	TScriptInterface<IAttackable> TargetInterface = EnemyAI->GetTarget();
	UObject* MovementNodes = TargetInterface.GetObject();




	if (MovementNodes == nullptr)
	{
		UE_LOG(LogTemp, Verbose, TEXT("UBTService_TargetLocation: TargetObject is null (EnemyAI has no current target)."));
		OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
		return;
	}
	OwnerComp.GetBlackboardComponent()->SetValueAsObject(GetSelectedBlackboardKey(), MovementNodes);
}
