#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "JJHUD.generated.h"

class UFont;
class AJJGameMode;
class AJJCharacter;

// HUD moderno (barras de salud, armadura y munición, armas, llaves, mira), mensajes,
// pantallas de fin de nivel y los menús de inicio, pausa y opciones gráficas.
UCLASS()
class JJDOOM_API AJJHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	// Fila del menú que está bajo el ratón (-1 = ninguna).
	int32 HoveredRow = -1;

private:
	void DrawGameHUD(AJJGameMode* GM, AJJCharacter* Player, float S);
	void DrawEndScreens(AJJGameMode* GM, float S);
	void DrawMenu(AJJGameMode* GM, float S);

	void DrawCentered(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale);
	void DrawShadowed(const FString& Text, const FLinearColor& Color, float X, float Y, UFont* Font, float Scale);
	void DrawRightAligned(const FString& Text, const FLinearColor& Color, float RightX, float Y, UFont* Font, float Scale);
	void DrawPanel(float X, float Y, float W, float H, const FLinearColor& Accent, float S);
	void DrawFrame(float X, float Y, float W, float H, float Thickness, const FLinearColor& Color);
	void DrawSegmentBar(float X, float Y, float W, float H, float Fraction, const FLinearColor& Color, int32 Segments, float S);
	void DrawVignette(const FLinearColor& Color, float Strength);

	FVector2D LastMouse = FVector2D(-1.f, -1.f);
	float SmoothFPS = 60.f;
};
