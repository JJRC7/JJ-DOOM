#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JJFlash.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

// Destello breve: fogonazos, impactos, sangre y explosiones (bola que crece + luz que se apaga).
UCLASS()
class JJDOOM_API AJJFlash : public AActor
{
	GENERATED_BODY()

public:
	AJJFlash();

	static AJJFlash* Spawn(UWorld* World, const FVector& Where, const FLinearColor& Color, float Size, float LightCandelas, float Life);

	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Ball;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> Light;

	float Life = 0.3f;
	float Age = 0.f;
	float BaseSize = 0.1f;
	float BaseIntensity = 0.f;
};
