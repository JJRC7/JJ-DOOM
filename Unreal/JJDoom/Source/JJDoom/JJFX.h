#pragma once

#include "CoreMinimal.h"

class UWorld;

// Efectos: usan las partículas y sonidos del Starter Content si están en el proyecto
// y, si no, destellos de luz propios.
namespace JJFX
{
	// Chispas al impactar una bala (en paredes o enemigos).
	void Sparks(UWorld* World, const FVector& Where, const FVector& Normal);

	// Explosión con fuego, luz y sonido (Scale 1 = tamaño de un barril).
	void Explosion(UWorld* World, const FVector& Where, float Scale);
}
