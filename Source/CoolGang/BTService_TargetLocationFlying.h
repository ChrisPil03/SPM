#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_TargetLocationFlying.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UBTService_TargetLocationFlying : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_TargetLocationFlying();
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};