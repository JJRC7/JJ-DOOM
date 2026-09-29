#include "JJFX.h"
#include "JJFlash.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Misc/PackageName.h"
#include "Engine/World.h"

namespace
{
	template <class T>
	T* JJFindStarterAsset(const TCHAR* Folder, const TCHAR* Name)
	{
		const FString Package = FString::Printf(TEXT("/Game/StarterContent/%s/%s"), Folder, Name);
		if (!FPackageName::DoesPackageExist(Package))
		{
			return nullptr;
		}
		return LoadObject<T>(nullptr, *FString::Printf(TEXT("%s.%s"), *Package, Name));
	}
}

void JJFX::Sparks(UWorld* World, const FVector& Where, const FVector& Normal)
{
	if (!World)
	{
		return;
	}
	static UParticleSystem* SparkSystem = nullptr;
	static bool bLooked = false;
	if (!bLooked)
	{
		bLooked = true;
		SparkSystem = JJFindStarterAsset<UParticleSystem>(TEXT("Particles"), TEXT("P_Sparks"));
		if (SparkSystem)
		{
			SparkSystem->AddToRoot();
		}
	}
	const FVector Point = Where + Normal * 4.f;
	if (SparkSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, SparkSystem, Point, Normal.Rotation(), FVector(0.35f), true);
	}
	// Chispazo de luz breve (sin sombras, barato).
	AJJFlash::Spawn(World, Point, FLinearColor(1.f, 0.7f, 0.25f), SparkSystem ? 0.f : 0.06f, 60.f, 0.12f);
}

void JJFX::Explosion(UWorld* World, const FVector& Where, float Scale)
{
	if (!World)
	{
		return;
	}
	static UParticleSystem* ExplosionSystem = nullptr;
	static USoundBase* ExplosionSound = nullptr;
	static bool bLooked = false;
	if (!bLooked)
	{
		bLooked = true;
		ExplosionSystem = JJFindStarterAsset<UParticleSystem>(TEXT("Particles"), TEXT("P_Explosion"));
		ExplosionSound = JJFindStarterAsset<USoundBase>(TEXT("Audio"), TEXT("Explosion_Cue"));
		if (ExplosionSystem)
		{
			ExplosionSystem->AddToRoot();
		}
		if (ExplosionSound)
		{
			ExplosionSound->AddToRoot();
		}
	}
	if (ExplosionSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, ExplosionSystem, Where, FRotator::ZeroRotator, FVector(Scale), true);
	}
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, ExplosionSound, Where, FMath::Clamp(0.5f + Scale * 0.3f, 0.5f, 1.2f));
	}
	AJJFlash::Spawn(World, Where, FLinearColor(1.f, 0.45f, 0.1f), ExplosionSystem ? 0.f : 2.2f * Scale, 2500.f * Scale, 0.5f);
}
