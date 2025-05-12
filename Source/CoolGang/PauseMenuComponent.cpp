#include "PauseMenuComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h" // Fix: Include for UUserWidget and CreateWidget

/*
	This did not work because a)
		Input is only sent once
		Executon of all code stops as soon as the game is paused..
		The variable for the UI is set to null every time the code is recompiled.
		
*/

UPauseMenuComponent::UPauseMenuComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPauseMenuComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPauseMenuComponent::TogglePauseMenu(bool paused)
{
	// Cast to Pawn
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	// Cast to PlayerController
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	if (!paused)
	{
		PlayerController->bShowMouseCursor = false;
	}
	else
	{
		PlayerController->bShowMouseCursor = true;
	}
}