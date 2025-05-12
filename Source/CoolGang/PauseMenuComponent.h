#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PauseMenuComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COOLGANG_API UPauseMenuComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPauseMenuComponent();
	// Expose TogglePauseMenu to Blueprints
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void TogglePauseMenu(bool paused);
protected:
	virtual void BeginPlay() override;
};