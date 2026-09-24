#include "JJTypes.h"

const FJJEnemyDef& FJJEnemyDef::Get(EJJEnemyKind Kind)
{
	// Vida, velocidad, escala, ataque, alcance, espera mín/máx, prob. de dolor,
	// cuerpo a cuerpo mín/máx, disparo mín/máx, vel. proyectil, color, color proyectil, vuela, jefe, fantasma
	static const FJJEnemyDef Defs[] = {
		{ 20.f,  450.f, 1.0f, EJJAttack::Hitscan,    5600.f, 1.2f, 2.6f, 0.75f,  4.f, 10.f,  3.f, 12.f,    0.f, FLinearColor(0.20f, 0.26f, 0.12f), FLinearColor(1.f, 0.8f, 0.3f),   false, false, false }, // Zombi
		{ 30.f,  450.f, 1.0f, EJJAttack::Shotgun,    4800.f, 1.4f, 2.8f, 0.70f,  4.f, 10.f,  3.f, 10.f,    0.f, FLinearColor(0.08f, 0.08f, 0.09f), FLinearColor(1.f, 0.8f, 0.3f),   false, false, false }, // Sargento
		{ 60.f,  500.f, 1.0f, EJJAttack::Projectile, 7200.f, 1.4f, 3.0f, 0.60f,  6.f, 16.f,  8.f, 22.f, 1400.f, FLinearColor(0.28f, 0.13f, 0.06f), FLinearColor(1.f, 0.35f, 0.05f), false, false, false }, // Imp
		{ 150.f, 900.f, 1.1f, EJJAttack::Melee,       200.f, 0.7f, 1.1f, 0.50f, 10.f, 28.f,  0.f,  0.f,    0.f, FLinearColor(0.60f, 0.20f, 0.26f), FLinearColor::Black,             false, false, false }, // Demonio
		{ 150.f, 900.f, 1.1f, EJJAttack::Melee,       200.f, 0.7f, 1.1f, 0.50f, 10.f, 28.f,  0.f,  0.f,    0.f, FLinearColor(0.10f, 0.05f, 0.07f), FLinearColor::Black,             false, false, true  }, // Espectro
		{ 40.f,  600.f, 0.8f, EJJAttack::Charge,     3600.f, 1.2f, 2.4f, 1.00f,  3.f, 20.f,  0.f,  0.f,    0.f, FLinearColor(0.80f, 0.74f, 0.60f), FLinearColor(1.f, 0.4f, 0.05f),  true,  false, false }, // Alma perdida
		{ 250.f, 400.f, 1.2f, EJJAttack::Projectile, 8000.f, 1.6f, 3.0f, 0.50f, 10.f, 30.f, 10.f, 30.f, 1200.f, FLinearColor(0.55f, 0.04f, 0.03f), FLinearColor(0.8f, 0.2f, 1.f),  true,  false, false }, // Cacodemonio
		{ 1000.f, 500.f, 1.5f, EJJAttack::Projectile, 8800.f, 1.0f, 2.2f, 0.20f, 15.f, 40.f, 15.f, 35.f, 1500.f, FLinearColor(0.62f, 0.40f, 0.32f), FLinearColor(0.2f, 1.f, 0.2f),  false, true,  false }, // Barón
	};
	return Defs[static_cast<int32>(Kind)];
}

const FJJWeaponDef& FJJWeaponDef::Get(EJJWeapon Weapon)
{
	static const FJJWeaponDef Defs[] = {
		{ TEXT("PISTOLA"),       EJJAmmo::Bullets, 0.38f, 1, 0.015f,  6.f, 15.f, false },
		{ TEXT("ESCOPETA"),      EJJAmmo::Shells,  0.95f, 7, 0.09f,   5.f, 15.f, false },
		{ TEXT("AMETRALLADORA"), EJJAmmo::Bullets, 0.11f, 1, 0.04f,   6.f, 15.f, false },
		{ TEXT("LANZACOHETES"),  EJJAmmo::Rockets, 0.80f, 1, 0.f,    20.f, 80.f, true  },
	};
	return Defs[static_cast<int32>(Weapon)];
}
