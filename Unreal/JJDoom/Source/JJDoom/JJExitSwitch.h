#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JJExitSwitch.generated.h"

class UStaticMeshComponent;

// Bloque de pared con el interruptor de SALIDA (franja roja; se vuelve verde al activarlo).
UCLASS()
class JJDOOM_API AJJExitSwitch : public AActor
{
	GENERATED_BODY()

public:
	AJJExitSwitch();
	virtual void BeginPlay() override;
	void Activate();

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Block;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Panel;

	bool bUsed = false;
};
