#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "JJTypes.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UAnimInstance;
class UAnimationAsset;

UENUM(BlueprintType)
enum class EJJWeapon : uint8 { Pistol, Shotgun, Chaingun, Rocket };

UENUM(BlueprintType)
enum class EJJAmmo : uint8 { Bullets, Shells, Rockets };

UENUM(BlueprintType)
enum class EJJEnemyKind : uint8 { Zombie, Sergeant, Imp, Demon, Spectre, LostSoul, Caco, Baron };

UENUM(BlueprintType)
enum class EJJPickup : uint8 { Medkit, Stim, Soul, Clip, Shells, Rockets, Armor, Shotgun, Chaingun, RocketLauncher, RedKey, BlueKey };

UENUM(BlueprintType)
enum class EJJKey : uint8 { None, Red, Blue };

UENUM(BlueprintType)
enum class EJJGameState : uint8 { Playing, Exiting, Intermission, Victory, Dead };

enum class EJJAttack : uint8 { Hitscan, Shotgun, Projectile, Melee, Charge };

enum class EJJEnemyState : uint8 { Idle, Chase, Attack, Pain, Charge, Dying, Dead };

// Estadísticas de cada enemigo. Distancias en unidades de Unreal (1 casilla = 400 = 4 m).
struct FJJEnemyDef
{
	float Health;
	float Speed;
	float Scale;
	EJJAttack Attack;
	float Range;
	float CooldownMin;
	float CooldownMax;
	float PainChance;
	float MeleeMin;
	float MeleeMax;
	float ShotMin;     // daño por disparo o proyectil
	float ShotMax;
	float ProjSpeed;
	FLinearColor Color;
	FLinearColor ProjColor;
	bool bFlying;
	bool bBoss;
	bool bGhost;

	static const FJJEnemyDef& Get(EJJEnemyKind Kind);
};

struct FJJWeaponDef
{
	const TCHAR* Name;
	EJJAmmo Ammo;
	float Cooldown;
	int32 Pellets;
	float Spread;      // radianes
	float DamageMin;
	float DamageMax;
	bool bRocket;

	static const FJJWeaponDef& Get(EJJWeapon Weapon);
};

// Salud, armadura, munición y armas del jugador (se guarda al empezar cada nivel).
struct FJJInventory
{
	float Health = 100.f;
	float Armor = 0.f;
	int32 Ammo[3] = { 50, 0, 0 };
	bool bHas[4] = { true, false, false, false };
	EJJWeapon Current = EJJWeapon::Pistol;
};

// Modelo 3D real para un tipo de enemigo (por ejemplo, descargado de Fab).
// Se asigna en un Blueprint hijo de JJGameMode, sin programar.
USTRUCT(BlueprintType)
struct FJJMonsterVisual
{
	GENERATED_BODY()

	// Malla con esqueleto del monstruo.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	// Opción A: su Animation Blueprint (si el paquete trae uno).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	TSubclassOf<UAnimInstance> AnimBlueprint;

	// Opción B: animaciones sueltas (caminar en bucle, atacar y morir).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	TObjectPtr<UAnimationAsset> WalkAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	TObjectPtr<UAnimationAsset> AttackAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	TObjectPtr<UAnimationAsset> DeathAnimation = nullptr;

	// Ajustes de tamaño y orientación del modelo.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	float Scale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	float YawOffset = -90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	float HeightOffset = 0.f;
};

// Modelo 3D real para un arma (malla estática o con esqueleto).
USTRUCT(BlueprintType)
struct FJJWeaponVisual
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	// Posición respecto a la cámara (X adelante, Y derecha, Z arriba), giro y escala.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	FVector Offset = FVector(35.f, 14.f, -22.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	float Scale = 1.f;
};
