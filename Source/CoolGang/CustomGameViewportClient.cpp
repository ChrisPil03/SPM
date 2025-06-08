#include "CustomGameViewportClient.h"
#include "Engine/Canvas.h"

void UCustomGameViewportClient::PostRender(UCanvas* Canvas)
{
        Super::PostRender(Canvas);

        if (bFading)
        {
                DrawScreenFade(Canvas);
        }
}

void UCustomGameViewportClient::ClearFade()
{
        bFading = false;
}

void UCustomGameViewportClient::Fade(const float Duration, const bool bInToBlack)
{
        if (World)
        {
                bFading = true;
                this->bToBlack = bInToBlack;
                FadeDuration = Duration;
                FadeStartTime = World->GetTimeSeconds();
        }
}

void UCustomGameViewportClient::DrawScreenFade(UCanvas* Canvas)
{
        if (bFading)
        {
                if (World)
                {
                        const float Time = World->GetTimeSeconds();
                        const float Alpha = FMath::Clamp((Time - FadeStartTime) / FadeDuration, 0.f, 1.f);

                        if (Alpha == 1.f && !bToBlack)
                        {
                                bFading = false;
                        }
                        else
                        {
                                FColor OldColor = Canvas->DrawColor;
                                FLinearColor FadeColor = FLinearColor::Black;
                                FadeColor.A = bToBlack ? Alpha : 1 - Alpha;
                                Canvas->DrawColor = FadeColor.ToFColor(true);
                                Canvas->DrawTile(Canvas->DefaultTexture, 0, 0, Canvas->ClipX, Canvas->ClipY, 0, 0, Canvas->DefaultTexture->GetSizeX(), Canvas->DefaultTexture->GetSizeY());
                                Canvas->DrawColor = OldColor;
                        }
                }
        }
}