#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EscapeSequenceGameInstance.generated.h"

UCLASS()
class COOLGANG_API UEscapeSequenceGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION()
	virtual void BeginLoadingScreen(const FString& MapName);
	UFUNCTION()
	virtual void EndLoadingScreen(UWorld* InLoadedWorld);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
};