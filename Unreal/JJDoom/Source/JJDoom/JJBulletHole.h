#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JJBulletHole.generated.h"

class UStaticMeshComponent;

// Marca de bala en la pared: un disco oscuro con un borde chamuscado. Dura un rato y luego desaparece.
UCLASS()
class JJDOOM_API AJJBulletHole : public AActor
{
	GENERATED_BODY()

public:
	AJJBulletHole();

	static void Spawn(UWorld* World, const FVector& Where, const FVector& Normal);

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Hole;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Scorch;
};
