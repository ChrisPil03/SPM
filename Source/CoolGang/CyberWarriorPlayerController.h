// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CyberWarriorPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API ACyberWarriorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void GameHasEnded(AActor* EndGameFocus, bool bIsWinner) override;

	UFUNCTION(BlueprintCallable)
	void RestartGame();
	
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> HUDClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> LoseScreenClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> WinScreenClass;
	
	UPROPERTY(EditAnywhere)
	float RestartDelay = 5;
	
	FTimerHandle RestartTimer;

	UPROPERTY()
	UUserWidget* HUD;
};
