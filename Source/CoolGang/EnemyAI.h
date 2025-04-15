#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthComponent.h"
#include "EnemyAI.generated.h"


UCLASS()
class COOLGANG_API AEnemyAI : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyAI();

	virtual void Tick(float DeltaTime) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	

	void Attack();

	float GetAttackRange() const;
	
private:
	UPROPERTY(EditAnywhere)
	class UCapsuleComponent* CapsuleComp;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(EditAnywhere)
	float AttackRange;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;
	
	void Die();
};
