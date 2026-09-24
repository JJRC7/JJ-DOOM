#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JJTypes.h"
#include "JJLevelBuilder.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UDirectionalLightComponent;
class USkyLightComponent;
class UExponentialHeightFogComponent;
class USkyAtmosphereComponent;
class AJJDoor;

// Construye el nivel a partir del mapa de texto: paredes, suelo, techo, luces, puertas, enemigos y objetos.
// También calcula el camino hacia el jugador para los enemigos que no lo ven.
UCLASS(Blueprintable)
class JJDOOM_API AJJLevelBuilder : public AActor
{
	GENERATED_BODY()

public:
	AJJLevelBuilder();

	static constexpr float Cell = 400.f;
	static constexpr float WallHeight = 400.f;

	void Build(int32 LevelIndex);
	void Clear();
	virtual void Tick(float DeltaSeconds) override;

	FVector GetPlayerStart() const { return PlayerStart; }
	float GetPlayerYaw() const { return PlayerYaw; }
	bool IsSolidCell(int32 X, int32 Y) const;
	FIntPoint WorldToCell(const FVector& Where) const;
	FVector CellCenter(int32 X, int32 Y, float Z = 0.f) const;
	bool NextStepTowardsPlayer(const FVector& From, FVector& OutTarget) const;

	int32 TotalEnemies = 0;
	int32 TotalItems = 0;

	// Materiales opcionales para un aspecto más realista (asígnalos en un Blueprint hijo).
	// El índice es el tipo de pared: 1 ladrillo, 2 metal, 3 piedra, 4 carne, 10 panel técnico, 11 madera, 12 mármol.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ|Materiales")
	TArray<TObjectPtr<UMaterialInterface>> WallMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ|Materiales")
	TObjectPtr<UMaterialInterface> FloorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ|Materiales")
	TObjectPtr<UMaterialInterface> CeilingMaterial;

	// Intensidad de las lámparas del techo (candelas).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ|Luz")
	float LampCandelas = 35.f;

private:
	UInstancedStaticMeshComponent* GetISM(int32 Key, const FLinearColor& Color, UMaterialInterface* Override, bool bCollision);
	void AddLamp(int32 X, int32 Y, const FLinearColor& Color);
	void ComputeFlow();

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> Sun;

	UPROPERTY()
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY()
	TObjectPtr<UExponentialHeightFogComponent> Fog;

	UPROPERTY()
	TObjectPtr<USkyAtmosphereComponent> Atmosphere;

	UPROPERTY()
	TMap<int32, TObjectPtr<UInstancedStaticMeshComponent>> ISMs;

	UPROPERTY()
	TArray<TObjectPtr<UActorComponent>> Lamps;

	UPROPERTY()
	TMap<int32, TObjectPtr<AJJDoor>> Doors;

	TArray<uint8> Grid;
	TArray<int32> Flow;
	int32 W = 0;
	int32 H = 0;
	FVector PlayerStart = FVector(200.f, 200.f, 110.f);
	float PlayerYaw = 0.f;
	float FlowTimer = 0.f;
};
