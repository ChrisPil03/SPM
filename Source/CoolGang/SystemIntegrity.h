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
	void SystemShutdown();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity", meta = (AllowPrivateAccess))
	float MaxIntegrity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integrity", meta = (AllowPrivateAccess))
	float CurrentIntegrity;
};
