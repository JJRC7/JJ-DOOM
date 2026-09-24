#include "JJPickup.h"
#include "JJAssets.h"
#include "JJCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AJJPickup::AJJPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Main = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Main"));
	Main->SetupAttachment(RootComponent);
	Detail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Detail"));
	Detail->SetupAttachment(RootComponent);
	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(RootComponent);
	Glow->SetCastShadows(false);
}

AJJPickup* AJJPickup::SpawnPickup(UWorld* World, EJJPickup InType, const FVector& Where, int32 InAmount, bool bInCounted)
{
	if (!World)
	{
		return nullptr;
	}
	const FTransform Transform(FRotator::ZeroRotator, Where);
	AJJPickup* P = World->SpawnActorDeferred<AJJPickup>(AJJPickup::StaticClass(), Transform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!P)
	{
		return nullptr;
	}
	P->Type = InType;
	P->Amount = InAmount;
	P->bCounted = bInCounted;
	P->FinishSpawning(Transform);
	return P;
}

void AJJPickup::BeginPlay()
{
	Super::BeginPlay();
	Base = GetActorLocation();
	Detail->SetVisibility(false);
	float GlowCd = 0.f;
	FLinearColor GlowColor = FLinearColor::White;
	using namespace JJAssets;
	switch (Type)
	{
	case EJJPickup::Medkit:
		SetupPart(Main, Cube(), FLinearColor(0.85f, 0.85f, 0.85f), FVector(0.5f, 0.35f, 0.3f), FVector::ZeroVector);
		SetupPart(Detail, Cube(), FLinearColor(0.8f, 0.02f, 0.02f), FVector(0.52f, 0.1f, 0.32f), FVector::ZeroVector);
		break;
	case EJJPickup::Stim:
		SetupPart(Main, Cube(), FLinearColor(0.85f, 0.85f, 0.85f), FVector(0.25f, 0.18f, 0.2f), FVector::ZeroVector);
		SetupPart(Detail, Cube(), FLinearColor(0.8f, 0.02f, 0.02f), FVector(0.27f, 0.06f, 0.21f), FVector::ZeroVector);
		break;
	case EJJPickup::Soul:
		SetupPart(Main, Sphere(), FLinearColor(0.2f, 0.4f, 1.f), FVector(0.5f), FVector(0.f, 0.f, 30.f));
		GlowCd = 40.f; GlowColor = FLinearColor(0.3f, 0.5f, 1.f);
		break;
	case EJJPickup::Clip:
		SetupPart(Main, Cube(), FLinearColor(0.45f, 0.35f, 0.08f), FVector(0.12f, 0.2f, 0.3f), FVector::ZeroVector);
		break;
	case EJJPickup::Shells:
		SetupPart(Main, Cube(), FLinearColor(0.5f, 0.05f, 0.04f), FVector(0.35f, 0.25f, 0.2f), FVector::ZeroVector);
		SetupPart(Detail, Cube(), FLinearColor(0.75f, 0.6f, 0.2f), FVector(0.3f, 0.2f, 0.22f), FVector(0.f, 0.f, 4.f));
		break;
	case EJJPickup::Rockets:
		SetupPart(Main, Cube(), FLinearColor(0.35f, 0.22f, 0.1f), FVector(0.5f, 0.3f, 0.3f), FVector::ZeroVector);
		SetupPart(Detail, Cylinder(), FLinearColor(0.6f, 0.6f, 0.6f), FVector(0.12f, 0.12f, 0.45f), FVector(0.f, 0.f, 20.f));
		break;
	case EJJPickup::Armor:
		SetupPart(Main, Cube(), FLinearColor(0.08f, 0.45f, 0.08f), FVector(0.55f, 0.3f, 0.6f), FVector(0.f, 0.f, 10.f));
		GlowCd = 6.f; GlowColor = FLinearColor(0.2f, 1.f, 0.2f);
		break;
	case EJJPickup::Shotgun:
		SetupPart(Main, Cylinder(), FLinearColor(0.06f, 0.06f, 0.06f), FVector(0.06f, 0.06f, 0.9f), FVector::ZeroVector, FRotator(90.f, 0.f, 0.f));
		SetupPart(Detail, Cube(), FLinearColor(0.3f, 0.15f, 0.06f), FVector(0.35f, 0.1f, 0.12f), FVector(-35.f, 0.f, 0.f));
		break;
	case EJJPickup::Chaingun:
		SetupPart(Main, Cylinder(), FLinearColor(0.08f, 0.08f, 0.08f), FVector(0.16f, 0.16f, 0.8f), FVector::ZeroVector, FRotator(90.f, 0.f, 0.f));
		SetupPart(Detail, Cube(), FLinearColor(0.15f, 0.15f, 0.15f), FVector(0.3f, 0.25f, 0.25f), FVector(-40.f, 0.f, 0.f));
		break;
	case EJJPickup::RocketLauncher:
		SetupPart(Main, Cylinder(), FLinearColor(0.15f, 0.2f, 0.1f), FVector(0.22f, 0.22f, 1.1f), FVector::ZeroVector, FRotator(90.f, 0.f, 0.f));
		SetupPart(Detail, Cube(), FLinearColor(0.7f, 0.55f, 0.02f), FVector(0.1f, 0.24f, 0.24f), FVector(10.f, 0.f, 0.f));
		break;
	case EJJPickup::RedKey:
	case EJJPickup::BlueKey:
	{
		const FLinearColor C = Type == EJJPickup::RedKey ? FLinearColor(0.9f, 0.03f, 0.03f) : FLinearColor(0.05f, 0.2f, 1.f);
		SetupPart(Main, Cube(), C, FVector(0.05f, 0.25f, 0.35f), FVector(0.f, 0.f, 20.f));
		GlowCd = 15.f; GlowColor = C;
		break;
	}
	}
	Glow->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	Glow->SetIntensityUnits(ELightUnits::Candelas);
	Glow->SetIntensity(GlowCd);
	Glow->SetAttenuationRadius(400.f);
	Glow->SetLightColor(GlowColor);
	Glow->SetVisibility(GlowCd > 0.f);
}

void AJJPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Time += DeltaSeconds;
	SetActorLocationAndRotation(Base + FVector(0.f, 0.f, FMath::Sin(Time * 2.5f) * 6.f), FRotator(0.f, Time * 90.f, 0.f));

	CheckTimer -= DeltaSeconds;
	if (CheckTimer > 0.f)
	{
		return;
	}
	CheckTimer = 0.1f;
	AJJCharacter* Player = Cast<AJJCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player)
	{
		return;
	}
	const FVector D = Player->GetActorLocation() - Base;
	if (FVector(D.X, D.Y, 0.f).Size() < 100.f && FMath::Abs(D.Z) < 220.f && Player->TryPickup(Type, Amount, bCounted))
	{
		Destroy();
	}
}
