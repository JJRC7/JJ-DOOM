#include "JJBarrel.h"
#include "JJAssets.h"
#include "JJFlash.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "TimerManager.h"
#include "Engine/World.h"

AJJBarrel::AJJBarrel()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Top = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Top"));
	Top->SetupAttachment(RootComponent);
	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(RootComponent);
	Glow->SetCastShadows(false);
}

void AJJBarrel::BeginPlay()
{
	Super::BeginPlay();
	JJAssets::SetupPart(Body, JJAssets::Cylinder(), FLinearColor(0.12f, 0.16f, 0.09f), FVector(0.7f, 0.7f, 1.1f), FVector(0.f, 0.f, 55.f));
	// Dinámico para que las explosiones (daño radial) lo encuentren y exploten en cadena.
	Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	JJAssets::SetupPart(Top, JJAssets::Cylinder(), FLinearColor(0.35f, 1.f, 0.05f), FVector(0.6f, 0.6f, 0.04f), FVector(0.f, 0.f, 111.f));
	Glow->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	Glow->SetIntensityUnits(ELightUnits::Candelas);
	Glow->SetIntensity(3.f);
	Glow->SetAttenuationRadius(300.f);
	Glow->SetLightColor(FLinearColor(0.4f, 1.f, 0.1f));
}

float AJJBarrel::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bPrimed || DamageAmount <= 0.f)
	{
		return 0.f;
	}
	Health -= DamageAmount;
	if (Health <= 0.f)
	{
		bPrimed = true;
		GetWorldTimerManager().SetTimer(BoomTimer, this, &AJJBarrel::Explode, 0.15f, false);
	}
	return DamageAmount;
}

void AJJBarrel::Explode()
{
	const FVector Where = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	AJJFlash::Spawn(GetWorld(), Where, FLinearColor(1.f, 0.5f, 0.1f), 2.6f, 3000.f, 0.6f);
	TArray<AActor*> Ignore;
	Ignore.Add(this);
	UGameplayStatics::ApplyRadialDamage(this, 130.f, Where, 1100.f, UDamageType::StaticClass(), Ignore, this, nullptr, false, ECC_Visibility);
	Destroy();
}
