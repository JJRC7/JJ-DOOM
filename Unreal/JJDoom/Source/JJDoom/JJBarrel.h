#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JJBarrel.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

// Barril tóxico: explota al recibir 20 de daño y hiere a todo lo que esté cerca.
UCLASS()
class JJDOOM_API AJJBarrel : public AActor
{
	GENERATED_BODY()

public:
	AJJBarrel();
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	void Explode();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Top;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> Glow;

	float Health = 20.f;
	bool bPrimed = false;
	FTimerHandle BoomTimer;
};
