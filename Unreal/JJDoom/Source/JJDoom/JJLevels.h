#pragma once

#include "CoreMinimal.h"

struct FJJLevelData
{
	FString Name;
	TArray<FString> Map;
	TArray<FIntRect> Sky;      // zonas al aire libre (casillas, ambos extremos incluidos)
	float Yaw = 0.f;           // orientación inicial del jugador
	bool bBoss = false;        // la salida no se abre hasta matar al Barón
	FLinearColor FloorColor = FLinearColor(0.1f, 0.1f, 0.1f);
	FLinearColor CeilingColor = FLinearColor(0.1f, 0.1f, 0.1f);
	FLinearColor LightColor = FLinearColor::White;
};

namespace JJLevels
{
	const TArray<FJJLevelData>& Get();
}
