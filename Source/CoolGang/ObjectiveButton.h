// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "ObjectiveButton.generated.h"

class AObjectivePressButtonsManager;

UCLASS()
class COOLGANG_API AObjectiveButton : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectiveButton();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Interact(AActor* Interactor) override;

public:
	bool GetIsPressed() const { return bPressed; }
	void ResetButton();
	
private:
	void PressButton();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	AObjectivePressButtonsManager* ObjectiveManager;
	
	bool bPressed = false;
};
