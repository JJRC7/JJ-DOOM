#pragma once

#include "CoreMinimal.h"

class UWorld;

// Efectos: usan las partículas y sonidos del Starter Content si están en el proyecto
// y, si no, destellos de luz propios.
namespace JJFX
{
	// Chispas amarillas breves al acertar a un enemigo.
	void Sparks(UWorld* World, const FVector& Where, const FVector& Normal);

	// Polvillo breve al impactar en una pared.
	void Dust(UWorld* World, const FVector& Where, const FVector& Normal);

	// Explosión con fuego, luz y sonido (Scale 1 = tamaño de un barril).
	void Explosion(UWorld* World, const FVector& Where, float Scale);
}
