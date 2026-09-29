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
	const FVector N = Normal.IsNearlyZero() ? FVector::UpVector : Normal.GetSafeNormal();
	const FVector Point = Where + N * 6.f;
	// Chispazo con luz (muy breve) y varias chispas amarillas que saltan, caen y se apagan.
	AJJFlash::Spawn(World, Point, FLinearColor(1.f, 0.8f, 0.15f), 0.06f, 150.f, 0.1f);
	for (int32 I = 0; I < 10; ++I)
	{
		if (AJJFlash* Spark = AJJFlash::Spawn(World, Point, FLinearColor(1.f, 0.85f, 0.05f), 0.03f, 0.f, FMath::FRandRange(0.18f, 0.35f)))
		{
			Spark->SetMotion(FMath::VRandCone(N, FMath::DegreesToRadians(70.f)) * FMath::FRandRange(350.f, 900.f), 1400.f);
		}
	}
}

void JJFX::Dust(UWorld* World, const FVector& Where, const FVector& Normal)
{
	if (!World)
	{
		return;
	}
	AJJFlash::Spawn(World, Where + Normal * 4.f, FLinearColor(0.3f, 0.28f, 0.25f), 0.07f, 0.f, 0.2f);
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
