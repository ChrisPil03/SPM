#include "FocusHandler.h"

void UFocusHandler::MoveMouseToWidget(const UWidget* Widget)
{
	if (!Widget)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveMouseToWidget: Widget is null."));
		return;
	}

	UWorld* World = Widget->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveMouseToWidget: World is null."));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveMouseToWidget: PlayerController is null."));
		return;
	}

	// Use GetPaintSpaceGeometry() instead of GetCachedGeometry()
	FGeometry Geometry = Widget->GetPaintSpaceGeometry();
	
	FVector2D LocalSize = Geometry.GetLocalSize();
	FVector2D LocalCenter = LocalSize * 0.5f;
	if (UVerticalBoxSlot* Slot = Cast<UVerticalBoxSlot>(Widget->Slot))
	{
		// Force layout refresh (optional)
		Slot->SynchronizeProperties();
	}
	FVector2D AbsoluteCenter = Geometry.LocalToAbsolute(LocalCenter);
	UE_LOG(LogTemp, Log, TEXT("AbsoluteCenter: (%.1f, %.1f)"), AbsoluteCenter.X, AbsoluteCenter.Y);

	// Get DPI scale
	float Scale = UWidgetLayoutLibrary::GetViewportScale(World);
	UE_LOG(LogTemp, Log, TEXT("Viewport Scale: %.2f"), Scale);
	if (Scale <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid viewport scale!"));
		return;
	}

	// Convert to viewport coordinates
	FVector2D ViewportPosition = AbsoluteCenter / Scale;

	// Ensure coordinates are within the viewport bounds
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	ViewportPosition.X = FMath::Clamp(ViewportPosition.X, 0.0f, ViewportSize.X);
	ViewportPosition.Y = FMath::Clamp(ViewportPosition.Y, 0.0f, ViewportSize.Y);

	PC->SetMouseLocation(ViewportPosition.X, ViewportPosition.Y);
	UE_LOG(LogTemp, Log, TEXT("Mouse moved to (%.1f, %.1f)"), ViewportPosition.X, ViewportPosition.Y);
}

bool UFocusHandler::HasWidgetFocus(const UWidget* Widget)
{
	if (!Widget)
	{
		return false;
	}

	// Get the Slate widget associated with this UWidget
	TSharedPtr<SWidget> SlateWidget = Widget->GetCachedWidget();
	if (!SlateWidget.IsValid())
	{
		return false;
	}

	// Get the currently focused slate widget for the first user (usually user 0)
	TSharedPtr<SWidget> FocusedWidget = FSlateApplication::Get().GetUserFocusedWidget(0);
	return FocusedWidget == SlateWidget;
}