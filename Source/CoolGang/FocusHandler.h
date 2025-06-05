// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Framework/Application/SlateApplication.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "Runtime/UMG/Public/Components/Widget.h"
#include "Components/VerticalBoxSlot.h"


#include "CoreMinimal.h"
#include "FocusHandler.generated.h"
/**
 * 
 */

class UButton;

UCLASS(Blueprintable)
class COOLGANG_API UFocusHandler : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Focus Utils")
	static void MoveMouseToWidget(const UWidget* Widget);
	UFUNCTION(BlueprintCallable, Category = "Focus Utils")
	static bool HasWidgetFocus(const UWidget* Widget);
	
	UFUNCTION(BlueprintCallable, Category = "Focus Utils")
	static void FindAllButtons(UWidget* RootWidget, TArray<UButton*>& OutButtons)
	{
		if (!RootWidget)
		{
			return;
		}

		// If this widget is a button, add it
		if (UButton* Button = Cast<UButton>(RootWidget))
		{
			OutButtons.Add(Button);
		}

		// If this widget is a panel, iterate its children
		if (UPanelWidget* Panel = Cast<UPanelWidget>(RootWidget))
		{
			int32 ChildCount = Panel->GetChildrenCount();
			for (int32 i = 0; i < ChildCount; ++i)
			{
				UWidget* Child = Panel->GetChildAt(i);
				FindAllButtons(Child, OutButtons);
			}
		}
	}

	UFUNCTION(BlueprintCallable, Category = "Focus Utils")
	// Helper for UserWidget: finds all buttons in the widget's root
	static void FindAllButtonsInWidget(UUserWidget* Widget, TArray<UButton*>& OutButtons)
	{
		if (!Widget)
		{
			return;
		}
		UWidget* RootWidget = Widget->GetRootWidget();
		FindAllButtons(RootWidget, OutButtons);
	}
	
};
