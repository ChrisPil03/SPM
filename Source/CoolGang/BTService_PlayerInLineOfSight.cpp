#include "BTService_PlayerInLineOfSight.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_PlayerInLineOfSight::UBTService_PlayerInLineOfSight()
{
	NodeName = "Update If Line Of Sight To Player";
}

void UBTService_PlayerInLineOfSight::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	APawn *PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return;
	}
	
	if (PlayerPawn == nullptr)
	{
		return;
	}
	
	if (OwnerComp.GetAIOwner()->LineOfSightTo(PlayerPawn))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), false);
	}
}
