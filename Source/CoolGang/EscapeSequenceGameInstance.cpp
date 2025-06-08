#include "EscapeSequenceGameInstance.h"

#include "CustomGameViewportClient.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"

void UEscapeSequenceGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEscapeSequenceGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UEscapeSequenceGameInstance::EndLoadingScreen);
}

void UEscapeSequenceGameInstance::BeginLoadingScreen(const FString& InMapName)
{
	if (!IsRunningDedicatedServer())
	{
		UCustomGameViewportClient* GameViewportClient = Cast<UCustomGameViewportClient>(GetGameViewportClient());
		if (GameViewportClient)
		{
			GameViewportClient->Fade(8, true); // Fade to black
		}
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.MoviePaths.Add(TEXT("IntroMovie"));
		LoadingScreen.MinimumLoadingScreenDisplayTime = 2.0f;
		LoadingScreen.PlaybackType = EMoviePlaybackType::MT_LoadingLoop;

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
}

void UEscapeSequenceGameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{
	UCustomGameViewportClient* GameViewportClient = Cast<UCustomGameViewportClient>(GetGameViewportClient());
	if (GameViewportClient)
	{
		GameViewportClient->Fade(8, false); // Fade from black
	}
}