#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"
#include "PlayerCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/SphereComponent.h"
#include "EnemyAI.generated.h"

class AObjectiveBase;
class UNiagaraComponent;
class UNiagaraSystem;
class UEnemySpawnManagerSubsystem;
class UCapsuleComponent;
class UStaticMeshComponent;
class AEnemyAIController;

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Spider UMETA(DisplayName = "Spider"),
	Wasp UMETA(DisplayName = "Wasp"),
	Gloorb UMETA(DisplayName = "Gloorb")
};

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

	UFUNCTION(BlueprintImplementableEvent)
	void PerformPreDeathActions();

	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetChanged(AActor* Target);
	
	void Die();
	
public:
	void Attack();

	void StartDeathSequence();
	
	UFUNCTION(BlueprintCallable)
	float GetAttackRange() const;
	
	UPROPERTY(EditAnywhere, Category = "Drop")
	TSubclassOf<AActor> Drop;

	static bool IsPlayerShieldActive(AActor* Player);
	
	UPROPERTY(EditAnywhere, Category = "Drop")
	float DropRate;

	UPROPERTY(EditAnywhere, Category = "Stats")
	const class UEnemyAttributeSet* EnemyAttributeSet;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere,  Category = "Sound", meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere,  Category = "Sound", meta = (AllowPrivateAccess = "true"))
	class USoundBase* MovementSound;
	

	UFUNCTION(BlueprintCallable)
	TScriptInterface<IAttackable> GetTarget() const;

	UFUNCTION()
	void AttackPlayer(AObjectiveBase*  Objective);

	void DropUpgrade();

	AActor* GetCurrentTarget() const;

	void SetCurrentTarget(AActor* Target);

	UBehaviorTree* GetBehaviorTree() const {return BehaviorTree;}

	UFUNCTION(BlueprintCallable)
	bool IsDead() const {return bIsDead;}
	
	UFUNCTION(BlueprintCallable)
	bool IsJumping() const {return bIsJumping;}
	
	UFUNCTION(BlueprintCallable)
	bool IsAttacking() const {return bIsAttacking;}

	UFUNCTION(BlueprintCallable)
	void SetAttackingState(bool IsAttacking) {bIsAttacking = IsAttacking;}

	UFUNCTION(BlueprintCallable)
	void SetJumpingState(bool IsJumping) {bIsJumping = IsJumping;}

private:
	UFUNCTION()
	void AttackObjective(AObjectiveBase* Objective);
	void GiveScore();
	
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

	bool bFadeComplete = true;
	bool bIsDead = false;
	bool bIsJumping = false;
	bool bIsAttacking = false;

	bool bDeathVFXComplete = false;

	UFUNCTION()
	void OnDeathFXFinished(UNiagaraComponent* PooledNiagaraComp);
	
	UFUNCTION()
	void OnFadeFinished();

	void ReleaseToPool();
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float AttackRange;
	
	UPROPERTY()
	UEnemySpawnManagerSubsystem* EnemySpawnManager;
	
	UPROPERTY(EditAnywhere)
	float AttackDamage;

	UPROPERTY(EditAnywhere)
	float Health;

	UPROPERTY(EditAnywhere)
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;
	
	UPROPERTY(VisibleAnywhere)
	TScriptInterface<IAttackable> CurrentTarget;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* MovementTarget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> AttackAbilityClass;

	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_ResetHealth;
	
	UPROPERTY(EditAnywhere, Category = "GameplayEffect Class")
	TSubclassOf<class UGameplayEffect> GE_InitEnemyStats;

	void InitEnemyStats();

	void GiveAbilities();

	UPROPERTY(VisibleAnywhere)
	bool bChangedToTargetPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	EEnemyType EnemyType;
};

