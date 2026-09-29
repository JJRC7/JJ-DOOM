#include "JJHUD.h"
#include "JJGameMode.h"
#include "JJCharacter.h"
#include "JJEnemy.h"
#include "JJLevels.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/App.h"

namespace
{
	const FLinearColor JJRed(1.f, 0.14f, 0.08f);
	const FLinearColor JJGold(1.f, 0.78f, 0.2f);
	const FLinearColor JJGrey(0.72f, 0.72f, 0.75f);
	const FLinearColor JJCyan(0.2f, 0.75f, 1.f);
	const int32 JJMaxAmmo[] = { 200, 50, 50 };

	FString JJTime(float Seconds)
	{
		const int32 T = FMath::FloorToInt(Seconds);
		return FString::Printf(TEXT("%d:%02d"), T / 60, T % 60);
	}

	FLinearColor WithAlpha(FLinearColor Color, float Alpha)
	{
		Color.A = Alpha;
		return Color;
	}
}

// ---------------------------------------------------------------------------------------------
// Utilidades de dibujo
// ---------------------------------------------------------------------------------------------

void AJJHUD::DrawCentered(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale)
{
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	DrawShadowed(Text, Color, (Canvas->ClipX - W) * 0.5f, Y, Font, Scale);
}

void AJJHUD::DrawShadowed(const FString& Text, const FLinearColor& Color, float X, float Y, UFont* Font, float Scale)
{
	const float Offset = FMath::Max(1.f, Scale);
	DrawText(Text, FLinearColor(0.f, 0.f, 0.f, 0.85f * Color.A), X + Offset, Y + Offset, Font, Scale);
	DrawText(Text, Color, X, Y, Font, Scale);
}

void AJJHUD::DrawRightAligned(const FString& Text, const FLinearColor& Color, float RightX, float Y, UFont* Font, float Scale)
{
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	DrawShadowed(Text, Color, RightX - W, Y, Font, Scale);
}

void AJJHUD::DrawFrame(float X, float Y, float W, float H, float T, const FLinearColor& Color)
{
	DrawRect(Color, X, Y, W, T);
	DrawRect(Color, X, Y + H - T, W, T);
	DrawRect(Color, X, Y + T, T, H - 2.f * T);
	DrawRect(Color, X + W - T, Y + T, T, H - 2.f * T);
}

void AJJHUD::DrawPanel(float X, float Y, float W, float H, const FLinearColor& Accent, float S)
{
	// Fondo oscuro translúcido con degradado vertical, borde fino y línea de color arriba.
	const int32 Steps = 8;
	for (int32 I = 0; I < Steps; ++I)
	{
		const float A = FMath::Lerp(0.72f, 0.5f, static_cast<float>(I) / (Steps - 1));
		DrawRect(FLinearColor(0.02f, 0.02f, 0.03f, A), X, Y + H * I / Steps, W, H / Steps + 1.f);
	}
	DrawFrame(X, Y, W, H, FMath::Max(1.f, S), FLinearColor(1.f, 1.f, 1.f, 0.08f));
	DrawRect(WithAlpha(Accent, 0.95f), X, Y, W, 3.f * S);
	DrawRect(WithAlpha(Accent, 0.25f), X, Y + 3.f * S, W, 5.f * S);
	// Esquinas marcadas, estilo militar.
	const float C = 12.f * S;
	const float T = 2.f * S;
	DrawRect(WithAlpha(Accent, 0.9f), X, Y + H - T, C, T);
	DrawRect(WithAlpha(Accent, 0.9f), X, Y + H - C, T, C);
	DrawRect(WithAlpha(Accent, 0.9f), X + W - C, Y + H - T, C, T);
	DrawRect(WithAlpha(Accent, 0.9f), X + W - T, Y + H - C, T, C);
}

void AJJHUD::DrawSegmentBar(float X, float Y, float W, float H, float Fraction, const FLinearColor& Color, int32 Segments, float S)
{
	// Barra por segmentos con degradado (oscuro a la izquierda, brillante a la derecha) y brillo arriba.
	Fraction = FMath::Clamp(Fraction, 0.f, 1.f);
	const float Gap = FMath::Max(1.f, 2.f * S);
	const float SegW = (W - Gap * (Segments - 1)) / Segments;
	for (int32 I = 0; I < Segments; ++I)
	{
		const float SX = X + I * (SegW + Gap);
		DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.07f), SX, Y, SegW, H);
		const float Fill = FMath::Clamp(Fraction * Segments - I, 0.f, 1.f);
		if (Fill <= 0.f)
		{
			continue;
		}
		const float T = Segments > 1 ? static_cast<float>(I) / (Segments - 1) : 1.f;
		FLinearColor C = FMath::Lerp(Color * 0.45f, Color, T);
		C.A = 1.f;
		DrawRect(C, SX, Y, SegW * Fill, H);
		DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.25f), SX, Y, SegW * Fill, FMath::Max(1.f, H * 0.3f));
	}
	// Resplandor bajo la barra.
	if (Fraction > 0.f)
	{
		DrawRect(WithAlpha(Color, 0.18f), X, Y + H, W * Fraction, 3.f * S);
	}
}

void AJJHUD::DrawVignette(const FLinearColor& Color, float Strength)
{
	// Bordes de la pantalla teñidos, con más intensidad cuanto más cerca del borde.
	Strength = FMath::Clamp(Strength, 0.f, 1.f);
	if (Strength <= 0.f)
	{
		return;
	}
	const float SW = Canvas->ClipX;
	const float SH = Canvas->ClipY;
	const int32 Rings = 12;
	const FLinearColor C = WithAlpha(Color, 0.07f * Strength);
	for (int32 I = 1; I <= Rings; ++I)
	{
		const float T = SH * 0.018f * I;
		DrawRect(C, 0.f, 0.f, SW, T);
		DrawRect(C, 0.f, SH - T, SW, T);
		DrawRect(C, 0.f, T, T, SH - 2.f * T);
		DrawRect(C, SW - T, T, T, SH - 2.f * T);
	}
}

// ---------------------------------------------------------------------------------------------
// HUD principal
// ---------------------------------------------------------------------------------------------

void AJJHUD::DrawHUD()
{
	Super::DrawHUD();
	AJJGameMode* GM = AJJGameMode::Get(this);
	if (!Canvas || !GM)
	{
		return;
	}
	GM->SyncMenuState();
	const float S = Canvas->ClipY / 720.f;
	const float Delta = static_cast<float>(FApp::GetDeltaTime());
	if (Delta > 0.f)
	{
		SmoothFPS = FMath::Lerp(SmoothFPS, 1.f / Delta, 0.05f);
	}

	const bool bMainMenu = GM->Menu == EJJMenu::Main || (GM->Menu == EJJMenu::Options && GM->MenuParent == EJJMenu::Main);
	if (!bMainMenu)
	{
		if (AJJCharacter* Player = Cast<AJJCharacter>(GetOwningPawn()))
		{
			DrawGameHUD(GM, Player, S);
		}
		DrawEndScreens(GM, S);
	}
	if (GM->Menu != EJJMenu::None)
	{
		DrawMenu(GM, S);
	}
	else
	{
		HoveredRow = -1;
	}
	if (GM->bShowFPS)
	{
		const int32 FPS = FMath::RoundToInt(SmoothFPS);
		const FLinearColor C = FPS >= 50 ? FLinearColor(0.3f, 1.f, 0.4f) : FPS >= 30 ? JJGold : JJRed;
		DrawShadowed(FString::Printf(TEXT("%d FPS"), FPS), C, 16.f * S, 12.f * S, GEngine->GetMediumFont(), 1.2f * S);
	}
}

void AJJHUD::DrawGameHUD(AJJGameMode* GM, AJJCharacter* Player, float S)
{
	const float SW = Canvas->ClipX;
	const float SH = Canvas->ClipY;
	const float Now = GetWorld()->GetRealTimeSeconds();
	UFont* Big = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();
	UFont* Small = GEngine->GetSmallFont();
	const FJJInventory& Inv = Player->Inv;

	// ---- Efectos a pantalla completa ----
	DrawVignette(FLinearColor(0.9f, 0.f, 0.f), Player->DamageFlash * 1.4f);
	DrawVignette(JJGold, Player->PickupFlash * 0.8f);
	if (!Player->IsDead() && Inv.Health < 30.f)
	{
		DrawVignette(FLinearColor(0.7f, 0.f, 0.f), 0.35f + 0.25f * FMath::Sin(Now * 5.f));
	}

	// ---- Mira: se pone roja sobre un enemigo y marca una X al acertar ----
	if (!Player->IsDead() && GM->State == EJJGameState::Playing && GM->Menu == EJJMenu::None)
	{
		bool bOnEnemy = false;
		if (APlayerController* PC = GetOwningPlayerController())
		{
			FVector Eye;
			FRotator Rot;
			PC->GetPlayerViewPoint(Eye, Rot);
			FHitResult Hit;
			FCollisionQueryParams Query(SCENE_QUERY_STAT(JJAim), false, Player);
			if (GetWorld()->LineTraceSingleByChannel(Hit, Eye, Eye + Rot.Vector() * 9000.f, ECC_Visibility, Query))
			{
				const AJJEnemy* Enemy = Cast<AJJEnemy>(Hit.GetActor());
				bOnEnemy = Enemy && Enemy->IsAlive();
			}
		}
		const float CX = SW * 0.5f;
		const float CY = SH * 0.5f;
		const float Gap = (bOnEnemy ? 4.f : 6.f) * S;
		const float Len = 8.f * S;
		const float T = FMath::Max(2.f, 2.f * S);
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			// Primera pasada: sombra; segunda: la mira.
			const float O = Pass == 0 ? 1.f : 0.f;
			const FLinearColor C = Pass == 0 ? FLinearColor(0.f, 0.f, 0.f, 0.5f)
				: bOnEnemy ? FLinearColor(1.f, 0.15f, 0.1f, 0.95f) : FLinearColor(1.f, 1.f, 1.f, 0.85f);
			DrawRect(C, CX - T * 0.5f + O, CY - Gap - Len + O, T, Len);
			DrawRect(C, CX - T * 0.5f + O, CY + Gap + O, T, Len);
			DrawRect(C, CX - Gap - Len + O, CY - T * 0.5f + O, Len, T);
			DrawRect(C, CX + Gap + O, CY - T * 0.5f + O, Len, T);
			DrawRect(C, CX - T * 0.5f + O, CY - T * 0.5f + O, T, T);
		}
		if (Player->HitMarker > 0.f)
		{
			const FLinearColor HC(1.f, 1.f, 1.f, FMath::Min(1.f, Player->HitMarker * 5.f));
			const float A = 6.f * S;
			const float B = 13.f * S;
			DrawLine(CX - B, CY - B, CX - A, CY - A, HC, T);
			DrawLine(CX + B, CY - B, CX + A, CY - A, HC, T);
			DrawLine(CX - B, CY + B, CX - A, CY + A, HC, T);
			DrawLine(CX + B, CY + B, CX + A, CY + A, HC, T);
		}
	}

	// ---- Panel izquierdo: salud y armadura ----
	{
		const float W = 360.f * S;
		const float H = 124.f * S;
		const float X = 24.f * S;
		const float Y = SH - H - 24.f * S;
		const float HP = Inv.Health;
		FLinearColor HC = HP > 60.f ? FLinearColor(0.2f, 1.f, 0.45f) : HP > 30.f ? FLinearColor(1.f, 0.72f, 0.1f) : JJRed;
		if (HP <= 30.f)
		{
			HC = FMath::Lerp(HC, FLinearColor::White, 0.3f * (0.5f + 0.5f * FMath::Sin(Now * 8.f)));
		}
		DrawPanel(X, Y, W, H, HC, S);

		// Icono de cruz médica.
		const float IX = X + 18.f * S;
		const float IY = Y + 16.f * S;
		const float IS = 22.f * S;
		DrawRect(HC, IX + IS * 0.35f, IY, IS * 0.3f, IS);
		DrawRect(HC, IX, IY + IS * 0.35f, IS, IS * 0.3f);
		DrawShadowed(TEXT("SALUD"), JJGrey, IX + IS + 10.f * S, IY + 2.f * S, Medium, 1.15f * S);
		DrawRightAligned(FString::FromInt(FMath::CeilToInt(HP)), FLinearColor::White, X + W - 16.f * S, Y + 6.f * S, Big, 1.55f * S);
		DrawSegmentBar(X + 18.f * S, Y + 48.f * S, W - 36.f * S, 18.f * S, HP / 100.f, HC, 20, S);
		if (HP > 100.f)
		{
			// Salud extra (por encima de 100): franja azul brillante sobre la barra.
			DrawRect(FLinearColor(0.5f, 0.85f, 1.f, 0.9f), X + 18.f * S, Y + 44.f * S, (W - 36.f * S) * FMath::Min(1.f, (HP - 100.f) / 100.f), 3.f * S);
		}

		// Icono de escudo y armadura.
		const float AY = Y + 78.f * S;
		const float AP = Inv.Armor;
		DrawRect(JJCyan, IX + 2.f * S, AY, 18.f * S, 12.f * S);
		DrawRect(JJCyan, IX + 6.f * S, AY + 12.f * S, 10.f * S, 5.f * S);
		DrawRect(JJCyan, IX + 9.f * S, AY + 17.f * S, 4.f * S, 3.f * S);
		DrawShadowed(TEXT("ARMADURA"), JJGrey, IX + IS + 10.f * S, AY - 2.f * S, Medium, 1.f * S);
		DrawRightAligned(FString::FromInt(FMath::CeilToInt(AP)), AP > 0.f ? FLinearColor::White : JJGrey * 0.7f, X + W - 16.f * S, AY - 6.f * S, Medium, 1.5f * S);
		DrawSegmentBar(X + 18.f * S, Y + 104.f * S, W - 36.f * S, 9.f * S, AP / 100.f, JJCyan, 20, S);
	}

	// ---- Panel derecho: arma, munición y armas disponibles ----
	{
		const float W = 380.f * S;
		const float H = 124.f * S;
		const float X = SW - W - 24.f * S;
		const float Y = SH - H - 24.f * S;
		const FJJWeaponDef& Weapon = FJJWeaponDef::Get(Inv.Current);
		const int32 AmmoType = static_cast<int32>(Weapon.Ammo);
		const int32 Ammo = Inv.Ammo[AmmoType];
		const int32 MaxAmmo = JJMaxAmmo[AmmoType];
		const float Fraction = static_cast<float>(Ammo) / MaxAmmo;
		const FLinearColor AC = Fraction > 0.25f ? FLinearColor(1.f, 0.55f, 0.1f) : JJRed;
		DrawPanel(X, Y, W, H, AC, S);

		DrawShadowed(Weapon.Name, JJGold, X + 18.f * S, Y + 12.f * S, Medium, 1.35f * S);

		// Casillas de armas 1-4.
		for (int32 I = 0; I < 4; ++I)
		{
			const float BX = X + 18.f * S + I * 36.f * S;
			const float BY = Y + 44.f * S;
			const float BS = 28.f * S;
			const bool bCurrent = static_cast<int32>(Inv.Current) == I;
			const bool bOwned = Inv.bHas[I];
			if (bCurrent)
			{
				DrawRect(WithAlpha(JJGold, 0.9f), BX, BY, BS, BS);
			}
			else
			{
				DrawRect(FLinearColor(1.f, 1.f, 1.f, bOwned ? 0.12f : 0.04f), BX, BY, BS, BS);
				DrawFrame(BX, BY, BS, BS, FMath::Max(1.f, S), FLinearColor(1.f, 1.f, 1.f, bOwned ? 0.5f : 0.12f));
			}
			const FString Num = FString::FromInt(I + 1);
			float TW = 0.f, TH = 0.f;
			GetTextSize(Num, TW, TH, Medium, 1.2f * S);
			const FLinearColor NC = bCurrent ? FLinearColor(0.08f, 0.05f, 0.f) : bOwned ? FLinearColor::White : FLinearColor(1.f, 1.f, 1.f, 0.25f);
			DrawText(Num, NC, BX + (BS - TW) * 0.5f, BY + (BS - TH) * 0.5f, Medium, 1.2f * S);
		}

		// Munición del arma actual: número grande, máximo y barra.
		FLinearColor NumColor = FLinearColor::White;
		if (Ammo == 0)
		{
			NumColor = WithAlpha(JJRed, 0.5f + 0.5f * FMath::Abs(FMath::Sin(Now * 6.f)));
		}
		else if (Fraction <= 0.25f)
		{
			NumColor = JJRed;
		}
		const FString MaxText = FString::Printf(TEXT("/%d"), MaxAmmo);
		float MW = 0.f, MH = 0.f;
		GetTextSize(MaxText, MW, MH, Medium, 1.1f * S);
		DrawShadowed(MaxText, JJGrey, X + W - 16.f * S - MW, Y + 30.f * S, Medium, 1.1f * S);
		DrawRightAligned(FString::FromInt(Ammo), NumColor, X + W - 20.f * S - MW, Y + 6.f * S, Big, 1.7f * S);
		DrawSegmentBar(X + 18.f * S, Y + 82.f * S, W - 36.f * S, 12.f * S, Fraction, AC, 25, S);

		// Reservas de cada tipo de munición.
		const TCHAR* Names[] = { TEXT("BALAS"), TEXT("CARTUCHOS"), TEXT("COHETES") };
		for (int32 I = 0; I < 3; ++I)
		{
			const FLinearColor C = I == AmmoType ? JJGold : JJGrey * 0.85f;
			DrawShadowed(FString::Printf(TEXT("%s %d"), Names[I], Inv.Ammo[I]), C, X + 18.f * S + I * 118.f * S, Y + 101.f * S, Small, 1.05f * S);
		}

		// Llaves encima del panel.
		float KX = X + W;
		auto DrawKey = [&](const TCHAR* Label, const FLinearColor& Color)
		{
			const float KW = 96.f * S;
			const float KH = 24.f * S;
			KX -= KW;
			const float KY = Y - KH - 8.f * S;
			DrawRect(WithAlpha(Color * 0.35f, 0.85f), KX, KY, KW, KH);
			DrawRect(Color, KX, KY, 5.f * S, KH);
			DrawFrame(KX, KY, KW, KH, FMath::Max(1.f, S), WithAlpha(Color, 0.9f));
			DrawShadowed(Label, FLinearColor::White, KX + 12.f * S, KY + 4.f * S, Small, 1.05f * S);
			KX -= 8.f * S;
		};
		if (Player->bBlueKey)
		{
			DrawKey(TEXT("LLAVE AZUL"), FLinearColor(0.15f, 0.4f, 1.f));
		}
		if (Player->bRedKey)
		{
			DrawKey(TEXT("LLAVE ROJA"), FLinearColor(1.f, 0.1f, 0.08f));
		}
	}

	// ---- Arriba a la derecha: enemigos y tiempo ----
	{
		const float W = 210.f * S;
		const float H = 58.f * S;
		const float X = SW - W - 24.f * S;
		const float Y = 20.f * S;
		DrawPanel(X, Y, W, H, JJRed, S);
		DrawShadowed(TEXT("ENEMIGOS"), JJGrey, X + 14.f * S, Y + 10.f * S, Small, 1.1f * S);
		DrawRightAligned(FString::Printf(TEXT("%d / %d"), GM->Kills, GM->TotalKills), FLinearColor::White, X + W - 14.f * S, Y + 8.f * S, Medium, 1.1f * S);
		DrawShadowed(TEXT("TIEMPO"), JJGrey, X + 14.f * S, Y + 32.f * S, Small, 1.1f * S);
		DrawRightAligned(JJTime(GM->LevelTime), FLinearColor::White, X + W - 14.f * S, Y + 30.f * S, Medium, 1.1f * S);
	}

	// ---- Mensajes arriba en el centro ----
	if (GM->MessageTime > 0.f && !GM->Message.IsEmpty())
	{
		const float Alpha = FMath::Clamp(GM->MessageTime * 2.f, 0.f, 1.f);
		float TW = 0.f, TH = 0.f;
		GetTextSize(GM->Message, TW, TH, Medium, 1.35f * S);
		const float PX = (SW - TW) * 0.5f - 20.f * S;
		const float PY = 60.f * S;
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f * Alpha), PX, PY, TW + 40.f * S, TH + 14.f * S);
		DrawRect(WithAlpha(JJRed, Alpha), PX, PY, 4.f * S, TH + 14.f * S);
		DrawRect(WithAlpha(JJRed, Alpha), PX + TW + 36.f * S, PY, 4.f * S, TH + 14.f * S);
		DrawShadowed(GM->Message, FLinearColor(1.f, 0.85f, 0.7f, Alpha), (SW - TW) * 0.5f, PY + 7.f * S, Medium, 1.35f * S);
	}
}

void AJJHUD::DrawEndScreens(AJJGameMode* GM, float S)
{
	const float SW = Canvas->ClipX;
	const float SH = Canvas->ClipY;
	UFont* Big = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();

	switch (GM->State)
	{
	case EJJGameState::Dead:
		DrawRect(FLinearColor(0.4f, 0.f, 0.f, FMath::Min(0.5f, GM->StateTime * 0.5f)), 0.f, 0.f, SW, SH);
		DrawCentered(TEXT("HAS MUERTO"), SH * 0.3f, JJRed, Big, 4.f * S);
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
		DrawCentered(JJLevels::Get()[GM->LevelIndex].Name, SH * 0.15f, JJGold, Medium, 1.8f * S);
		DrawCentered(TEXT("NIVEL COMPLETADO"), SH * 0.22f, JJRed, Big, 3.f * S);
		DrawCentered(FString::Printf(TEXT("ENEMIGOS  %d%%"), PK), SH * 0.4f, FLinearColor::White, Medium, 1.8f * S);
		DrawCentered(FString::Printf(TEXT("OBJETOS   %d%%"), PItems), SH * 0.47f, FLinearColor::White, Medium, 1.8f * S);
		DrawCentered(TEXT("TIEMPO    ") + JJTime(GM->LevelTime), SH * 0.54f, FLinearColor::White, Medium, 1.8f * S);
		DrawCentered(TEXT("Pulsa ENTER para continuar"), SH * 0.7f, JJGold, Medium, 1.5f * S);
		break;
	}
	case EJJGameState::Victory:
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.75f), 0.f, 0.f, SW, SH);
		DrawCentered(TEXT("¡VICTORIA!"), SH * 0.25f, JJGold, Big, 4.f * S);
		DrawCentered(TEXT("Has derrotado al Barón y limpiado el infierno de JJ."), SH * 0.42f, FLinearColor::White, Medium, 1.6f * S);
		DrawCentered(TEXT("Pulsa ENTER para jugar de nuevo"), SH * 0.6f, JJGold, Medium, 1.5f * S);
		break;
	default:
		break;
	}
}

// ---------------------------------------------------------------------------------------------
// Menús
// ---------------------------------------------------------------------------------------------

void AJJHUD::DrawMenu(AJJGameMode* GM, float S)
{
	const float SW = Canvas->ClipX;
	const float SH = Canvas->ClipY;
	const float Now = GetWorld()->GetRealTimeSeconds();
	UFont* Big = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();
	UFont* Small = GEngine->GetSmallFont();
	const bool bMain = GM->Menu == EJJMenu::Main || (GM->Menu == EJJMenu::Options && GM->MenuParent == EJJMenu::Main);

	// Fondo: el nivel congelado, oscurecido y con un tinte rojo abajo.
	DrawRect(FLinearColor(0.f, 0.f, 0.f, bMain ? 0.5f : 0.62f), 0.f, 0.f, SW, SH);
	for (int32 I = 0; I < 10; ++I)
	{
		DrawRect(FLinearColor(0.35f, 0.02f, 0.f, 0.035f * I), 0.f, SH * (0.6f + 0.04f * I), SW, SH * 0.04f + 1.f);
	}
	DrawVignette(FLinearColor::Black, 1.f);

	// Título.
	if (bMain)
	{
		const FString Title = TEXT("JJ DOOM");
		const float Pulse = 0.85f + 0.15f * FMath::Sin(Now * 2.f);
		float TW = 0.f, TH = 0.f;
		GetTextSize(Title, TW, TH, Big, 5.f * S);
		const float TX = (SW - TW) * 0.5f;
		const float TY = SH * 0.06f;
		// Resplandor rojo alrededor de las letras.
		for (int32 I = 0; I < 8; ++I)
		{
			const float Angle = I * UE_PI / 4.f;
			DrawText(Title, FLinearColor(1.f, 0.05f, 0.f, 0.18f * Pulse), TX + FMath::Cos(Angle) * 5.f * S, TY + FMath::Sin(Angle) * 5.f * S, Big, 5.f * S);
		}
		DrawText(Title, FLinearColor(0.f, 0.f, 0.f, 0.9f), TX + 4.f * S, TY + 4.f * S, Big, 5.f * S);
		DrawText(Title, FLinearColor(1.f, 0.2f * Pulse, 0.05f), TX, TY, Big, 5.f * S);
		DrawCentered(TEXT("EL  INFIERNO  TE  ESPERA"), TY + TH + 4.f * S, JJGold, Medium, 1.3f * S);
	}
	FString Heading;
	if (GM->Menu == EJJMenu::Pause)
	{
		Heading = TEXT("PAUSA");
	}
	else if (GM->Menu == EJJMenu::Options)
	{
		Heading = TEXT("OPCIONES GRÁFICAS");
	}
	if (!Heading.IsEmpty())
	{
		DrawCentered(Heading, bMain ? SH * 0.3f : SH * 0.14f, FLinearColor::White, Big, (bMain ? 1.4f : 2.6f) * S);
	}

	// Filas del menú.
	TArray<EJJMenuItem> Items;
	GM->GetMenuItems(Items);
	const float RowW = 580.f * S;
	const float RowH = 46.f * S;
	const float RowGap = 8.f * S;
	const float RX = (SW - RowW) * 0.5f;
	const float StartY = bMain ? SH * 0.4f : SH * 0.3f;

	// Ratón: la fila bajo el cursor se selecciona al moverlo.
	float MX = -1.f, MY = -1.f;
	HoveredRow = -1;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->GetMousePosition(MX, MY);
	}
	const bool bMouseMoved = FVector2D::DistSquared(FVector2D(MX, MY), LastMouse) > 1.f;
	LastMouse = FVector2D(MX, MY);
	for (int32 I = 0; I < Items.Num(); ++I)
	{
		const float RY = StartY + I * (RowH + RowGap);
		if (MX >= RX && MX <= RX + RowW && MY >= RY && MY <= RY + RowH)
		{
			HoveredRow = I;
			if (bMouseMoved)
			{
				GM->MenuIndex = I;
			}
		}
	}

	for (int32 I = 0; I < Items.Num(); ++I)
	{
		const float RY = StartY + I * (RowH + RowGap);
		const bool bSel = I == GM->MenuIndex;
		if (bSel)
		{
			// Selección: degradado rojo de izquierda a derecha.
			const int32 Slices = 16;
			for (int32 J = 0; J < Slices; ++J)
			{
				const float A = FMath::Lerp(0.9f, 0.15f, static_cast<float>(J) / (Slices - 1));
				DrawRect(FLinearColor(0.7f, 0.06f, 0.03f, A), RX + RowW * J / Slices, RY, RowW / Slices + 1.f, RowH);
			}
			DrawFrame(RX, RY, RowW, RowH, FMath::Max(1.f, S), FLinearColor(1.f, 0.4f, 0.2f, 0.6f));
			DrawRect(WithAlpha(JJGold, 0.7f + 0.3f * FMath::Sin(Now * 6.f)), RX, RY, 6.f * S, RowH);
		}
		else
		{
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f), RX, RY, RowW, RowH);
			DrawRect(FLinearColor(0.5f, 0.05f, 0.03f, 0.8f), RX, RY, 3.f * S, RowH);
		}

		const EJJMenuItem Item = Items[I];
		const FString Label = GM->GetMenuLabel(Item);
		float TW = 0.f, TH = 0.f;
		GetTextSize(Label, TW, TH, Medium, 1.4f * S);
		const float TY = RY + (RowH - TH) * 0.5f;
		DrawShadowed(Label, bSel ? FLinearColor::White : FLinearColor(0.75f, 0.75f, 0.78f), RX + (bSel ? 30.f : 24.f) * S, TY, Medium, 1.4f * S);

		const FString Value = GM->GetMenuValue(Item);
		if (!Value.IsEmpty())
		{
			const FString Shown = bSel ? FString::Printf(TEXT("<   %s   >"), *Value) : Value;
			DrawRightAligned(Shown, bSel ? JJGold : FLinearColor(0.85f, 0.65f, 0.3f), RX + RowW - 22.f * S, TY, Medium, 1.4f * S);
		}
	}

	// Explicación de la opción elegida y teclas.
	if (Items.IsValidIndex(GM->MenuIndex))
	{
		const float HintY = StartY + Items.Num() * (RowH + RowGap) + 14.f * S;
		DrawCentered(GM->GetMenuHint(Items[GM->MenuIndex]), HintY, FLinearColor(0.9f, 0.85f, 0.75f), Medium, 1.15f * S);
	}
	DrawCentered(TEXT("W/S o flechas: elegir     A/D: cambiar     ENTER o clic: aceptar     P / ESC: volver"),
		SH - 40.f * S, JJGrey, Small, 1.1f * S);
}
