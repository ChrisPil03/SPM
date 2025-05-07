#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HealthComponent.h"
#include "AbilitySystemComponent.h"
#include "PlayerCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "EnemyAI.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
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

	UHealthComponent* GetHealthComponent() const;

	UPROPERTY(EditAnywhere, Category = "Drop")
	TSubclassOf<AActor> Drop;
	
	UPROPERTY(EditAnywhere, Category = "Drop")
	float DropRate;

	UPROPERTY(EditAnywhere, Category = "Stats")
	const class UEnemyAttributeSet* EnemyAttributeSet;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UFUNCTION(BlueprintCallable)
	TScriptInterface<IAttackable> GetTarget() const;
	
	void Die();

	UFUNCTION()
	void AttackPlayer(AObjectiveBase*  Objective);

	void DropUpgrade();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TScriptInterface<IAttackable> CurrentTarget;

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

	UPROPERTY(EditAnywhere, Category="VFX")
	UMaterialInterface* FadeMaterial;
	
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* FadeDMI;
	
	float DeathStartTime;
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	float FadeDuration = 1.0f;

	bool bFadeComplete = true;
	bool bIsDead = false;

	bool bDeathVFXComplete = false;

	UFUNCTION()
	void OnDeathFXFinished(UNiagaraComponent* PooledNiagaraComp);
	
	UFUNCTION()
	void OnFadeFinished();

	void ReleaseToPool();
	
	UPROPERTY(EditAnywhere)
	float AttackRange;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

	UPROPERTY()
	UEnemySpawnManagerSubsystem* EnemySpawnManager;
	
	UPROPERTY(EditAnywhere)
	float AttackDamage;

	UPROPERTY(EditAnywhere)
	float Health;

	UPROPERTY(EditAnywhere)
	float MaxHealth;



	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> AttackAbilityClass;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> GE_ApplyDamageToPlayer;
	
	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_ResetHealth;
	
	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_InitEnemyStats;

	void InitEnemyStats();

	void GiveAbilities();

	UPROPERTY(VisibleAnywhere)
	bool bChangedToTargetPlayer;
};

