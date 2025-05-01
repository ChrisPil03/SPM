#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthComponent.h"
#include "AbilitySystemComponent.h"
#include "PlayerCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "EnemyAI.generated.h"

class UEnemySpawnManagerSubsystem;
class UCapsuleComponent;
class UStaticMeshComponent;
class AEnemyAIController;

UCLASS()
class COOLGANG_API AEnemyAI : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyAI();

	virtual void Tick(float DeltaTime) override;

	void SetAlive();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void Attack();

	float GetAttackRange() const;

	UHealthComponent *GetHealthComponent() const;

	UPROPERTY()
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

	ECollisionEnabled::Type CollisionType;

	UPROPERTY()
	AEnemyAIController* AIController;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, Category="VFX")
	UNiagaraSystem* DeathVFX;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* FadeDMI;

	float DeathStartTime;
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	float FadeDuration = 1.0f;

	bool bFadeComplete = false;

	bool bDeathVFXComplete = false;

	UFUNCTION()
	void OnDeathFXFinished(UNiagaraComponent* PooledNiagaraComp);
	
	UFUNCTION()
	void OnFadeFinished();

	void ReleaseToPool();
	
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
