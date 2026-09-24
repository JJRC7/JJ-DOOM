#include "JJHUD.h"
#include "JJGameMode.h"
#include "JJCharacter.h"
#include "JJLevels.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"

void AJJHUD::DrawCentered(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale)
{
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	DrawText(Text, FLinearColor::Black, (Canvas->ClipX - W) * 0.5f + 2.f, Y + 2.f, Font, Scale);
	DrawText(Text, Color, (Canvas->ClipX - W) * 0.5f, Y, Font, Scale);
}

void AJJHUD::DrawStat(const FString& Label, const FString& Value, float X, float Y, float S, const FLinearColor& Color)
{
	DrawText(Value, Color, X, Y, GEngine->GetLargeFont(), 2.2f * S);
	DrawText(Label, FLinearColor(0.8f, 0.8f, 0.8f), X, Y + 52.f * S, GEngine->GetSmallFont(), 1.2f * S);
}

static FString JJTime(float Seconds)
{
	const int32 T = FMath::FloorToInt(Seconds);
	return FString::Printf(TEXT("%d:%02d"), T / 60, T % 60);
}

void AJJHUD::DrawHUD()
{
	Super::DrawHUD();
	AJJGameMode* GM = AJJGameMode::Get(this);
	if (!Canvas || !GM)
	{
		return;
	}
	const float SW = Canvas->ClipX;
	const float SH = Canvas->ClipY;
	const float S = SH / 720.f;
	UFont* Big = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();
	const FLinearColor Red(1.f, 0.12f, 0.08f);
	const FLinearColor Gold(1.f, 0.8f, 0.2f);

	if (AJJCharacter* Player = Cast<AJJCharacter>(GetOwningPawn()))
	{
		if (Player->DamageFlash > 0.f)
		{
			DrawRect(FLinearColor(0.8f, 0.f, 0.f, Player->DamageFlash * 0.5f), 0.f, 0.f, SW, SH);
		}
		if (Player->PickupFlash > 0.f)
		{
			DrawRect(FLinearColor(1.f, 0.85f, 0.2f, Player->PickupFlash * 0.25f), 0.f, 0.f, SW, SH);
		}
		if (!Player->IsDead())
		{
			const float C = 7.f * S;
			DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.7f), SW * 0.5f - 1.f, SH * 0.5f - C, 2.f, C * 2.f);
			DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.7f), SW * 0.5f - C, SH * 0.5f - 1.f, C * 2.f, 2.f);
		}

		const FJJInventory& Inv = Player->Inv;
		const FJJWeaponDef& Weapon = FJJWeaponDef::Get(Inv.Current);
		const float BarH = 100.f * S;
		const float Y = SH - BarH;
		DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.7f), 0.f, Y, SW, BarH);
		DrawRect(FLinearColor(0.35f, 0.05f, 0.03f, 0.9f), 0.f, Y, SW, 3.f * S);
		const float Top = Y + 14.f * S;
		DrawStat(TEXT("MUNICIÓN"), FString::FromInt(Inv.Ammo[static_cast<int32>(Weapon.Ammo)]), SW * 0.05f, Top, S, Red);
		DrawStat(TEXT("SALUD"), FString::Printf(TEXT("%d%%"), FMath::CeilToInt(Inv.Health)), SW * 0.2f, Top, S, Red);
		DrawStat(TEXT("ARMADURA"), FString::Printf(TEXT("%d%%"), FMath::CeilToInt(Inv.Armor)), SW * 0.36f, Top, S, Red);
		DrawText(Weapon.Name, Gold, SW * 0.53f, Top + 10.f * S, Medium, 1.6f * S);
		FString Slots;
		for (int32 I = 0; I < 4; ++I)
		{
			Slots += Inv.bHas[I] ? FString::Printf(TEXT("%d "), I + 1) : FString(TEXT("- "));
		}
		DrawText(TEXT("ARMAS  ") + Slots, FLinearColor(0.8f, 0.8f, 0.8f), SW * 0.53f, Top + 52.f * S, GEngine->GetSmallFont(), 1.2f * S);
		const float AX = SW * 0.74f;
		DrawText(FString::Printf(TEXT("BAL %3d/200"), Inv.Ammo[0]), Gold, AX, Top, Medium, 1.1f * S);
		DrawText(FString::Printf(TEXT("CAR %3d/ 50"), Inv.Ammo[1]), Gold, AX, Top + 24.f * S, Medium, 1.1f * S);
		DrawText(FString::Printf(TEXT("COH %3d/ 50"), Inv.Ammo[2]), Gold, AX, Top + 48.f * S, Medium, 1.1f * S);
		if (Player->bRedKey)
		{
			DrawRect(FLinearColor(0.9f, 0.05f, 0.05f), SW * 0.92f, Top, 18.f * S, 28.f * S);
		}
		if (Player->bBlueKey)
		{
			DrawRect(FLinearColor(0.1f, 0.3f, 1.f), SW * 0.92f + 26.f * S, Top, 18.f * S, 28.f * S);
		}
	}

	if (GM->MessageTime > 0.f)
	{
		DrawText(GM->Message, FLinearColor::Black, 22.f * S, 22.f * S, Medium, 1.4f * S);
		DrawText(GM->Message, FLinearColor(1.f, 0.3f, 0.2f), 20.f * S, 20.f * S, Medium, 1.4f * S);
	}

	switch (GM->State)
	{
	case EJJGameState::Dead:
		DrawRect(FLinearColor(0.4f, 0.f, 0.f, FMath::Min(0.5f, GM->StateTime * 0.5f)), 0.f, 0.f, SW, SH);
		DrawCentered(TEXT("HAS MUERTO"), SH * 0.3f, Red, Big, 4.f * S);
		if (GM->StateTime > 1.f)
		{
			DrawCentered(TEXT("Pulsa ENTER o dispara para reintentar"), SH * 0.45f, FLinearColor::White, Medium, 1.5f * S);
		}
		break;
	case EJJGameState::Intermission:
	{
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.75f), 0.f, 0.f, SW, SH);
		const int32 PK = GM->TotalKills > 0 ? FMath::RoundToInt(100.f * GM->Kills / GM->TotalKills) : 100;
		const int32 PItems = GM->TotalItems > 0 ? FMath::RoundToInt(100.f * GM->Items / GM->TotalItems) : 100;
		DrawCentered(JJLevels::Get()[GM->LevelIndex].Name, SH * 0.15f, Gold, Medium, 1.8f * S);
		DrawCentered(TEXT("NIVEL COMPLETADO"), SH * 0.22f, Red, Big, 3.f * S);
		DrawCentered(FString::Printf(TEXT("ENEMIGOS  %d%%"), PK), SH * 0.4f, FLinearColor::White, Medium, 1.8f * S);
		DrawCentered(FString::Printf(TEXT("OBJETOS   %d%%"), PItems), SH * 0.47f, FLinearColor::White, Medium, 1.8f * S);
		DrawCentered(TEXT("TIEMPO    ") + JJTime(GM->LevelTime), SH * 0.54f, FLinearColor::White, Medium, 1.8f * S);
		DrawCentered(TEXT("Pulsa ENTER para continuar"), SH * 0.7f, Gold, Medium, 1.5f * S);
		break;
	}
	case EJJGameState::Victory:
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.75f), 0.f, 0.f, SW, SH);
		DrawCentered(TEXT("¡VICTORIA!"), SH * 0.25f, Gold, Big, 4.f * S);
		DrawCentered(TEXT("Has derrotado al Barón y limpiado el infierno de JJ."), SH * 0.42f, FLinearColor::White, Medium, 1.6f * S);
		DrawCentered(TEXT("Pulsa ENTER para jugar de nuevo"), SH * 0.6f, Gold, Medium, 1.5f * S);
		break;
	default:
		break;
	}
}
