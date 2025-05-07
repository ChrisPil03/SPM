#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gate.generated.h"

class UBoxComponent;
class UMoverComponent;

UCLASS()
class COOLGANG_API AGate : public AActor
{
	GENERATED_BODY()
	
public:	
	AGate();

protected:
	virtual void BeginPlay() override;

public:	
	void OpenGate() const;
	void CloseGate() const;

private:
	UPROPERTY(EditAnywhere, Category = "Gate")
	UStaticMeshComponent* GateMesh;

	UPROPERTY(EditAnywhere, Category = "Gate")
	UStaticMeshComponent* LeftDoorMesh;

	UPROPERTY(EditAnywhere, Category = "Gate")
	UStaticMeshComponent* RightDoorMesh;

	UPROPERTY(EditAnywhere, Category = "Gate")
	UMoverComponent* LeftMover;

	UPROPERTY(EditAnywhere, Category = "Gate")
	UMoverComponent* RightMover;

	UPROPERTY(EditAnywhere, Category = "Gate")
	UBoxComponent* BoxComponent;
};
