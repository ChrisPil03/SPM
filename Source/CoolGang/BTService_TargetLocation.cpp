#include "BTService_TargetLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "EnemyAI.h"

UBTService_TargetLocation::UBTService_TargetLocation()
{
	NodeName = "Update Player Location";
}

void UBTService_TargetLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UObject* Target = Cast<AEnemyAI>(OwnerComp.GetAIOwner()->GetPawn())->GetTarget().GetObject();
	
	if (Target == nullptr)
	{
		return;
	}
	
	OwnerComp.GetBlackboardComponent()->SetValueAsObject(GetSelectedBlackboardKey(), Target);
}

