#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "JJHUD.generated.h"

class UFont;

// Barra inferior (munición, salud, armadura, arma, llaves), mira, mensajes y pantallas de fin de nivel.
UCLASS()
class JJDOOM_API AJJHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawCentered(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale);
	void DrawStat(const FString& Label, const FString& Value, float X, float Y, float S, const FLinearColor& Color);
};
