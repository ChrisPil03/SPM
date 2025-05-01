#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthComponent.h"
#include "AbilitySystemComponent.h"
#include "PlayerCharacter.h"
#include "EnemyAI.generated.h"

class UEnemySpawnManagerSubsystem;
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

	UPROPERTY(EditAnywhere, Category = "Stats")
	const class UEnemyAttributeSet *EnemyAttributeSet;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent *AbilitySystemComponent;

	UFUNCTION(BlueprintCallable)
	TScriptInterface<IAttackable> GetTarget() const;
	
	void Die();

	UFUNCTION()
	void AttackPlayer(AObjectiveBase*  Objective);

private:
	UFUNCTION()
	void AttackObjective(AObjectiveBase* Objective);


	
	UPROPERTY(EditAnywhere)
	float AttackRange;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent *HealthComponent;

	UPROPERTY()
	UEnemySpawnManagerSubsystem* EnemySpawnManager;
	
	UPROPERTY(EditAnywhere)
	float AttackDamage;

	UPROPERTY(EditAnywhere)
	float Health;

	UPROPERTY(EditAnywhere)
	float MaxHealth;

	UPROPERTY(VisibleAnywhere)
	TScriptInterface<IAttackable> CurrentTarget;
	
	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_InitEnemyStats;

	UPROPERTY(VisibleAnywhere)
	bool bChangedToTargetPlayer;
};
