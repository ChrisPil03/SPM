#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericAbility.generated.h"

UCLASS()
class COOLGANG_API UGenericAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// Constructor
	UGenericAbility();

private:
	EGameplayAbilityInstancingPolicy::Type InstancingPolicy;
	
protected:

};