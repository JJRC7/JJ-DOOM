#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JJTypes.h"
#include "JJGameMode.generated.h"

class AJJLevelBuilder;
class AJJCharacter;
class AJJEnemy;
class UStaticMesh;
class UMaterialInterface;

// Controla la partida: carga los niveles, cuenta enemigos y objetos, la salida, la muerte y la victoria.
UCLASS()
class JJDOOM_API AJJGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJJGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	static AJJGameMode* Get(const UObject* WorldContext);

	// Clase del constructor de niveles (cámbiala por un Blueprint hijo para usar tus propios materiales).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JJ")
	TSubclassOf<AJJLevelBuilder> BuilderClass;

	void ShowMessage(const FString& Text, float Seconds = 3.f);
	void AddKill(AJJEnemy* Enemy);
	void AddItem() { Items++; }
	void AlertEnemies(const FVector& Where, float Radius);
	bool TryExit();
	void OnPlayerDied();
	void OnContinuePressed();
	void PlacePlayer(AJJCharacter* Player);
	AJJLevelBuilder* GetBuilder() const { return Builder; }

	EJJGameState State = EJJGameState::Playing;
	FString Message;
	float MessageTime = 0.f;
	float StateTime = 0.f;
	float LevelTime = 0.f;
	int32 LevelIndex = 0;
	int32 Kills = 0;
	int32 TotalKills = 0;
	int32 Items = 0;
	int32 TotalItems = 0;

private:
	void StartLevel(int32 Index, bool bNewGame);

	UPROPERTY()
	TObjectPtr<AJJLevelBuilder> Builder;

	// Referencias fijas a las formas básicas para que se incluyan al empaquetar el juego.
	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> BasicMeshes;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BasicMaterial;

	FJJInventory Snapshot;
	float ExitTime = 0.f;
};
