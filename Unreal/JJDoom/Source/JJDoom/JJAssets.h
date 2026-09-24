#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

// Formas básicas incluidas en el motor: el juego funciona sin importar ningún modelo.
namespace JJAssets
{
	UStaticMesh* Cube();
	UStaticMesh* Sphere();
	UStaticMesh* Cylinder();
	UStaticMesh* Cone();

	// Material básico del motor con el color indicado.
	UMaterialInstanceDynamic* ColorMaterial(UObject* Outer, const FLinearColor& Color);

	// Configura una pieza decorativa (sin colisión): forma, color, escala, posición y giro.
	void SetupPart(UStaticMeshComponent* Part, UStaticMesh* Mesh, const FLinearColor& Color, const FVector& Scale,
		const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
}
