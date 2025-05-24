
#include "CyberWarriorPlayerController.h"

#include "DiveGameMode.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

void ACyberWarriorPlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);
	HUD->RemoveFromParent();
	FlushPressedKeys();
	if (bIsWinner)
	{
		UUserWidget* WinScreen = CreateWidget(this, WinScreenClass);
	
		if (WinScreen != nullptr)
		{
			WinScreen->AddToViewport();

			SetInputMode(FInputModeUIOnly());
			SetShowMouseCursor(true);
		}
	}
	else
	{
		UUserWidget* LoseScreen = CreateWidget(this, LoseScreenClass);
	
		if (LoseScreen != nullptr)
		{
			LoseScreen->AddToViewport();
			
			SetInputMode(FInputModeUIOnly());
			SetShowMouseCursor(true);
		}
	}

}

void ACyberWarriorPlayerController::BeginPlay()
{
	Super::BeginPlay();
	HUD = CreateWidget(this, HUDClass);
	if (HUD != nullptr)
	{
		HUD->AddToViewport();
	}
}
