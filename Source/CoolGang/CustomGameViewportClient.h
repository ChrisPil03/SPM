#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "CustomGameViewportClient.generated.h"

UCLASS()
class COOLGANG_API UCustomGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	virtual void PostRender(UCanvas* Canvas) override;
	virtual void ClearFade();
	virtual void Fade(const float Duration, const bool bToBlack);
	void DrawScreenFade(UCanvas* Canvas);

private:
	uint32 bFading : 1;
	uint32 bToBlack : 1;
	float FadeAlpha;
	float FadeStartTime;
	float FadeDuration;
};