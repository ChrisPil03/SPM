
#include "CyberWarriorPlayerController.h"

#include "DiveGameMode.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ACyberWarriorPlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);
	HUD->RemoveFromParent();
	ADiveGameMode* GameMode = Cast<ADiveGameMode>(UGameplayStatics::GetGameMode(this));
	float TimeLeft = GameMode->GetTimeUntilNextObjective();
	if (TimeLeft <= 0)
	{
		UUserWidget* OutOfTimeScreen = CreateWidget(this, OutOfTimeScreenClass);
	
		if (OutOfTimeScreen != nullptr)
		{
			OutOfTimeScreen->AddToViewport();
		}
	}
	
	else if (bIsWinner)
	{
		UUserWidget* WinScreen = CreateWidget(this, WinScreenClass);
	
		if (WinScreen != nullptr)
		{
			WinScreen->AddToViewport();
		}
	}
	else
	{
		UUserWidget* LoseScreen = CreateWidget(this, LoseScreenClass);
	
		if (LoseScreen != nullptr)
		{
			LoseScreen->AddToViewport();
		}
	}
	GetWorldTimerManager().SetTimer(RestartTimer, this, &APlayerController::RestartLevel, RestartDelay);
}

void ACyberWarriorPlayerController::RestartGame()
{
	Super::RestartLevel();
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
