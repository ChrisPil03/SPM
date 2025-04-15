// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_PlayerInRange.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UBTService_PlayerInRange : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_PlayerInRange();
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
