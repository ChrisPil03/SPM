#include "BTService_TargetLocationFlying.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "EnemyAI.h"

UBTService_TargetLocationFlying::UBTService_TargetLocationFlying()
{
    NodeName = "Update Target Location In Air";
}

void UBTService_TargetLocationFlying::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
    
    if (!TargetInterface.GetInterface())
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
        return;
    }

    UObject* TargetObject = TargetInterface.GetObject();
    if (!TargetObject)
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
        return;
    }

    AActor* TargetActor = Cast<AActor>(TargetObject);
    if (!TargetActor)
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
        return;
    }

    TArray<USphereComponent*> MovementNodes = TargetInterface->GetMovementNodes();

    if (MovementNodes.Num() == 0)
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, MovementNodes.Num() - 1);
    USphereComponent* MovementNode = nullptr;

    if (MovementNodes.IsValidIndex(RandomIndex))
    {
        MovementNode = MovementNodes[RandomIndex];
    }

    if (MovementNode == nullptr)
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
        return;
    }
    
    OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), MovementNode->GetComponentLocation());
}
