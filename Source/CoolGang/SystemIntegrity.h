#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SystemIntegrity.generated.h"

UCLASS()
class COOLGANG_API ASystemIntegrity : public AActor
{
	GENERATED_BODY()
	
public:	
	ASystemIntegrity();

protected:
	virtual void BeginPlay() override;

public:
	void WeakenIntegrity(const float Damage);
	void StrengthenIntegrity(const float Integrity);

private:
	UFUNCTION()
	void SystemShutdown();
	
	UPROPERTY(EditAnywhere, Category = "Integrity")
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, Category = "Integrity")
	float MaxIntegrity;

	UPROPERTY(EditAnywhere, Category = "Integrity")
	float CurrentIntegrity;
};
