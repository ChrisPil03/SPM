#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "ObjectiveDefendGenerator.generated.h"

class AInteractableObject;
class AObjectiveDefendGenerator;
class UHealthComponent;

UCLASS()
class COOLGANG_API AObjectiveDefendGenerator : public AObjectiveBase
{
	GENERATED_BODY()
	
public:	
	AObjectiveDefendGenerator();

protected:
	virtual void BeginPlay() override;
	virtual void CompleteObjective() override;

public:	
	virtual void ResetObjective() override;
	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

private:
	void BindControlPanel();
	void BindDeathFunction();
	void BindCompletionFunction();
	void RegisterControlPanelInteraction(AInteractableObject* InteractableObject);

	void InitiateQuarantine();
	bool CanNotTakeDamage();

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(EditInstanceOnly, meta = (AllowPrivateAccess))
	AInteractableObject* ControlPanel;
};
