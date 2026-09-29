#include "JJBulletHole.h"
#include "JJAssets.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

namespace
{
	// Se guardan las últimas marcas para no llenar el nivel (las más viejas se borran).
	TArray<TWeakObjectPtr<AJJBulletHole>> GJJHoles;
	const int32 JJMaxHoles = 80;
}

AJJBulletHole::AJJBulletHole()
{
	InitialLifeSpan = 30.f;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	auto MakePart = [this](const TCHAR* Name)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(RootComponent);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCastShadow(false);
		return Part;
	};
	Scorch = MakePart(TEXT("Scorch"));
	Hole = MakePart(TEXT("Hole"));
}

void AJJBulletHole::Spawn(UWorld* World, const FVector& Where, const FVector& Normal)
{
	if (!World || Normal.IsNearlyZero())
	{
		return;
	}
	// El cilindro del motor es vertical (eje Z): se gira para que quede plano sobre la pared.
	const FRotator Facing = FRotationMatrix::MakeFromZ(Normal.GetSafeNormal()).Rotator();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AJJBulletHole* Mark = World->SpawnActor<AJJBulletHole>(AJJBulletHole::StaticClass(), Where + Normal * 0.4f, Facing, Params);
	if (!Mark)
	{
		return;
	}
	const float Size = FMath::FRandRange(0.8f, 1.2f);
	JJAssets::SetupPart(Mark->Scorch, JJAssets::Cylinder(), FLinearColor(0.04f, 0.035f, 0.03f), FVector(0.09f * Size, 0.09f * Size, 0.004f), FVector::ZeroVector);
	JJAssets::SetupPart(Mark->Hole, JJAssets::Cylinder(), FLinearColor(0.005f, 0.005f, 0.005f), FVector(0.045f * Size, 0.045f * Size, 0.006f), FVector(0.f, 0.f, 0.1f));

	GJJHoles.RemoveAll([](const TWeakObjectPtr<AJJBulletHole>& H) { return !H.IsValid(); });
	GJJHoles.Add(Mark);
	while (GJJHoles.Num() > JJMaxHoles)
	{
		if (GJJHoles[0].IsValid())
		{
			GJJHoles[0]->Destroy();
		}
		GJJHoles.RemoveAt(0);
	}
}
