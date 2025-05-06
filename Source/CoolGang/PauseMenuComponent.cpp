#include "PauseMenuComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h" // Fix: Include for UUserWidget and CreateWidget

UPauseMenuComponent::UPauseMenuComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize member variables
	PauseMenuClass = nullptr;
	PauseMenuInstance = nullptr;
	bIsPaused = false;
	bCanTogglePause = true; // Initialize the debounce flag
}

void UPauseMenuComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPauseMenuComponent::TogglePauseMenu()
{
	if (!bCanTogglePause) return; // Prevent rapid toggling
	bCanTogglePause = false; // Block further toggles temporarily

	// Debug log to confirm function is called
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("TogglePauseMenu called"));
	}
	// Cast to Pawn
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	// Cast to PlayerController
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	if (bIsPaused)
	{
		// Resume the game
		bIsPaused = false;
		UGameplayStatics::SetGamePaused(GetWorld(), false);

		if (PauseMenuInstance)
		{
			PauseMenuInstance->RemoveFromParent();
			PauseMenuInstance = nullptr;
		}

		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}
	else
	{
		// Pause the game
		bIsPaused = true;
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		if (!PauseMenuInstance && PauseMenuClass)
		{
			PauseMenuInstance = CreateWidget<UUserWidget>(PlayerController, PauseMenuClass);
			if (PauseMenuInstance)
			{
				PauseMenuInstance->AddToViewport();
			}
		}

		PlayerController->SetInputMode(FInputModeUIOnly());
		PlayerController->bShowMouseCursor = true;
		
	}

	// Re-enable toggling after a short delay
	GetWorld()->GetTimerManager().SetTimer(TogglePauseTimerHandle, this, &UPauseMenuComponent::EnableTogglePause, 0.2f, false);
	
}
void UPauseMenuComponent::EnableTogglePause()
{
	UE_LOG(LogTemp, Warning, TEXT("Can Toggle Again"));
	bCanTogglePause = true; // Allow toggling again
}