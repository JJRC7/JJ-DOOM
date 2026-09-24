#include "JJFlash.h"
#include "JJAssets.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"

AJJFlash::AJJFlash()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Ball = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball"));
	Ball->SetupAttachment(RootComponent);
	Ball->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ball->SetCastShadow(false);
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);
	Light->SetCastShadows(false);
}

AJJFlash* AJJFlash::Spawn(UWorld* World, const FVector& Where, const FLinearColor& Color, float Size, float LightCandelas, float Life)
{
	if (!World)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AJJFlash* Flash = World->SpawnActor<AJJFlash>(AJJFlash::StaticClass(), Where, FRotator::ZeroRotator, Params);
	if (!Flash)
	{
		return nullptr;
	}
	Flash->Life = FMath::Max(0.05f, Life);
	Flash->BaseSize = Size;
	Flash->BaseIntensity = LightCandelas;
	JJAssets::SetupPart(Flash->Ball, JJAssets::Sphere(), Color, FVector(Size), FVector::ZeroVector);
	Flash->Ball->SetVisibility(Size > 0.01f);
	Flash->Light->SetIntensityUnits(ELightUnits::Candelas);
	Flash->Light->SetIntensity(LightCandelas);
	Flash->Light->SetAttenuationRadius(FMath::Max(300.f, Size * 1500.f));
	Flash->Light->SetLightColor(Color);
	Flash->Light->SetVisibility(LightCandelas > 0.f);
	return Flash;
}

void AJJFlash::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;
	const float A = Age / Life;
	if (A >= 1.f)
	{
		Destroy();
		return;
	}
	Ball->SetRelativeScale3D(FVector(BaseSize * (1.f + A * 1.5f)));
	Light->SetIntensity(BaseIntensity * (1.f - A));
}
