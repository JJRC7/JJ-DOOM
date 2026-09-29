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

	// Dificultad: multiplica la vida de los enemigos y el daño que hacen.
	// Ahora lo controla el menú de dificultad (Fácil, Normal, Difícil, Pesadilla).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JJ|Dificultad")
	float EnemyHealthMultiplier = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JJ|Dificultad")
	float EnemyDamageMultiplier = 1.3f;

	// Modelos 3D reales para los monstruos (tipo de enemigo -> modelo). Vacío = formas básicas.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JJ|Modelos")
	TMap<EJJEnemyKind, FJJMonsterVisual> MonsterVisuals;

	// Modelos 3D reales para las armas.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JJ|Modelos")
	TMap<EJJWeapon, FJJWeaponVisual> WeaponVisuals;

	const FJJMonsterVisual* FindMonsterVisual(EJJEnemyKind Kind) const;
	const FJJWeaponVisual* FindWeaponVisual(EJJWeapon Weapon) const;

	void ShowMessage(const FString& Text, float Seconds = 3.f);
	void AddKill(AJJEnemy* Enemy);
	void AddItem() { Items++; }
	void AlertEnemies(const FVector& Where, float Radius);
	bool TryExit();
	void OnPlayerDied();
	void OnContinuePressed();
	void PlacePlayer(AJJCharacter* Player);
	AJJLevelBuilder* GetBuilder() const { return Builder; }

	// ---- Menús (inicio, pausa y opciones gráficas) ----
	void OpenMenu(EJJMenu NewMenu);
	void CloseMenu();
	void TogglePause();
	void MenuBack();
	void MenuMove(int32 Dir);
	void MenuAdjust(int32 Dir);
	void MenuAccept();
	void GetMenuItems(TArray<EJJMenuItem>& Out) const;
	FString GetMenuLabel(EJJMenuItem Item) const;
	FString GetMenuValue(EJJMenuItem Item) const;
	FString GetMenuHint(EJJMenuItem Item) const;
	// Mantiene la pausa y el cursor del ratón de acuerdo con el menú abierto.
	void SyncMenuState();

	// ---- Ajustes (se guardan en GameUserSettings.ini) ----
	static const TCHAR* DifficultyName(int32 Level);
	void SetDifficulty(int32 Level);
	int32 GetBrightness() const;
	void SetBrightness(int32 Level);
	void SetScreenPercentage(int32 Percent);
	void SaveOptions() const;

	EJJMenu Menu = EJJMenu::None;
	EJJMenu MenuParent = EJJMenu::Main;
	int32 MenuIndex = 0;
	int32 Difficulty = 1;
	int32 ScreenPercentage = 85;
	bool bShowFPS = false;

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
	void LoadOptions();
	void RestartLevel();

	// El nivel de fondo del menú principal está recién creado (Jugar no necesita reconstruirlo).
	bool bFreshLevel = false;
	int32 SavedBrightness = -1;

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
