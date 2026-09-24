#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JJTypes.h"
#include "JJDoor.generated.h"

class UStaticMeshComponent;
class AJJCharacter;

// Puerta que sube hasta el techo al usarla (E). Puede pedir la llave roja o la azul.
UCLASS()
class JJDOOM_API AJJDoor : public AActor
{
	GENERATED_BODY()

public:
	AJJDoor();

	void Setup(EJJKey InKey, bool bAlongX);
	bool TryOpen(AJJCharacter* By);
	bool IsPassable() const { return OpenAlpha > 0.6f; }

	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Panel;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Band;

	EJJKey Key = EJJKey::None;
	float OpenAlpha = 0.f;
	bool bOpening = false;
};
