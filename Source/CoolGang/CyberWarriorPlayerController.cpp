
#include "CyberWarriorPlayerController.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

void ACyberWarriorPlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);
	SetInputMode(FInputModeUIOnly());
	FlushPressedKeys();
}

void ACyberWarriorPlayerController::DisplayResultScreen()
{
	UUserWidget* LoseScreen = CreateWidget(this, LoseScreenClass);
	
	if (LoseScreen != nullptr)
	{
		LoseScreen->AddToViewport();
		SetShowMouseCursor(true);
	}
}

void ACyberWarriorPlayerController::BeginPlay()
{
	Super::BeginPlay();
	HUD = CreateWidget(this, HUDClass);
	if (HUD != nullptr)
	{
		HUD->AddToViewport();
		OnHUDReadyDelegate.Broadcast();
	}
}
