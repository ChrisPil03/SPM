#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_TargetLocationGround.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UBTService_TargetLocationGround : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_TargetLocationGround();
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category = "Targeting Logic")
	bool bPreferNavMeshProjectionForAirborneTarget;

	UPROPERTY(EditAnywhere, Category = "Targeting Logic", meta = (EditCondition = "bPreferNavMeshProjectionForAirborneTarget"))
	float NavMeshProjectionVerticalThreshold;

	UPROPERTY(EditAnywhere, Category = "Targeting Logic", meta = (EditCondition = "bPreferNavMeshProjectionForAirborneTarget"))
	float NavMeshProjectionHorizontalExtent;
	
	UPROPERTY(EditAnywhere, Category = "Targeting Logic|Offset")
	bool bApplyRandomOffset;
	
	UPROPERTY(EditAnywhere, Category = "Targeting Logic|Offset", meta = (EditCondition = "bApplyRandomOffset"))
	float MaxRandomOffsetRadius;
	
	UPROPERTY(EditAnywhere, Category = "TraceSettings")
	float GroundTraceDistance; // For fallback trace from player's movement nodes

	UPROPERTY(EditAnywhere, Category = "TraceSettings")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel;

	UPROPERTY(EditAnywhere, Category = "TraceSettings", meta = (DisplayName = "Draw Debug Trace"))
	bool bDrawDebugTraceForDuration;

	UPROPERTY(EditAnywhere, Category = "TraceSettings", meta = (EditCondition = "bDrawDebugTraceForDuration"))
	float DebugTraceDuration;
};