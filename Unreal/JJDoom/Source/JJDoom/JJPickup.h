#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JJTypes.h"
#include "JJPickup.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

// Botiquines, munición, armas, armadura y llaves. Giran y flotan; se recogen al pasar por encima.
UCLASS()
class JJDOOM_API AJJPickup : public AActor
{
	GENERATED_BODY()

public:
	AJJPickup();

	static AJJPickup* SpawnPickup(UWorld* World, EJJPickup Type, const FVector& Where, int32 Amount, bool bCounted);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	EJJPickup Type = EJJPickup::Medkit;
	int32 Amount = 0;        // 0 = cantidad normal
	bool bCounted = true;    // cuenta para el % de objetos del nivel

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Main;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Detail;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> Glow;

	FVector Base = FVector::ZeroVector;
	float Time = 0.f;
	float CheckTimer = 0.f;
};
