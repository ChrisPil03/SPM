#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthComponent.h"
#include "EnemyAI.generated.h"

class AEnemySpawnManager;
class UCapsuleComponent;
class UStaticMeshComponent;

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

	UHealthComponent* GetHealthComponent() const;
	
private:
	UPROPERTY(EditAnywhere)
	UCapsuleComponent* CapsuleComp;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(EditAnywhere)
	float AttackRange;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere)
	AEnemySpawnManager* EnemySpawnManager;

	UPROPERTY(EditAnywhere)
	float AttackDamage;

	
	void Die();
};
