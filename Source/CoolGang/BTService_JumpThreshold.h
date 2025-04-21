// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_JumpThreshold.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UBTService_JumpThreshold : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_JumpThreshold();

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	float JumpThreshold = 1000.f;
};
