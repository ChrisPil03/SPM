#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthComponent.h"
#include "AbilitySystemComponent.h"
#include "PlayerCharacter.h"
#include "EnemyAI.generated.h"

class AEnemySpawnManager;
class UCapsuleComponent;
class UStaticMeshComponent;

UCLASS()
class COOLGANG_API AEnemyAI : public ACharacter
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

	UHealthComponent *GetHealthComponent() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	class UEnemyAttributeSet *EnemyAttributeSet;

	void Die();

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent *BaseMesh;

	UPROPERTY(EditAnywhere)
	float AttackRange;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent *HealthComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AEnemySpawnManager> EnemySpawnManagerClass;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	AEnemySpawnManager *EnemySpawnManager;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent *AbilitySystemComponent;

	UPROPERTY(EditAnywhere)
	float AttackDamage;

	UPROPERTY(EditAnywhere)
	float Health;

	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_InitEnemyStats;
};
