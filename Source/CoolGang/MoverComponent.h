#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MoverComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOLGANG_API UMoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMoverComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetShouldMove(const bool bNewState) { ShouldMove = bNewState; }
	void SetMeshComponentToMove(UStaticMeshComponent* MeshComponent);
	void ReverseDirection(const bool bBackToOriginal);

private:
	void MoveObject(const float DeltaTime);
	
	UPROPERTY(EditAnywhere)
	bool ShouldMove;

	UPROPERTY(EditAnywhere)
	float MoveSpeed;

	UPROPERTY(EditAnywhere)
	FVector MoveOffset;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* MeshComponentToMove;

	FVector OriginalPosition;
	FVector TargetLocation;
};