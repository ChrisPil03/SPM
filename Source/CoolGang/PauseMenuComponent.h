#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PauseMenuComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COOLGANG_API UPauseMenuComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY()
	UUserWidget* PauseMenuInstance;

	bool bIsPaused = false;
	bool bCanTogglePause = true; // Debounce flag

	FTimerHandle TogglePauseTimerHandle;

public:
	UPauseMenuComponent();

	// Expose TogglePauseMenu to Blueprints
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void TogglePauseMenu();

protected:
	virtual void BeginPlay() override;

	void EnableTogglePause();
	
};