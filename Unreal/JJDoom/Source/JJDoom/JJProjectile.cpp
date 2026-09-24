#include "JJProjectile.h"
#include "JJAssets.h"
#include "JJFlash.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

AJJProjectile::AJJProjectile()
{
	InitialLifeSpan = 6.f;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(14.f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetNotifyRigidBodyCollision(true);
	Collision->OnComponentHit.AddDynamic(this, &AJJProjectile::OnHit);
	RootComponent = Collision;

	Ball = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball"));
	Ball->SetupAttachment(Collision);
	Ball->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ball->SetCastShadow(false);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Collision);
	Light->SetCastShadows(false);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->ProjectileGravityScale = 0.f;
	Movement->bRotationFollowsVelocity = true;
}

AJJProjectile* AJJProjectile::Fire(UWorld* World, AActor* Shooter, const FVector& From, const FVector& Direction, float InDamage,
	float Speed, const FLinearColor& InColor, bool bInExplosive, float Size)
{
	if (!World)
	{
		return nullptr;
	}
	const FTransform Transform(Direction.Rotation(), From);
	AJJProjectile* P = World->SpawnActorDeferred<AJJProjectile>(AJJProjectile::StaticClass(), Transform, Shooter, Cast<APawn>(Shooter),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!P)
	{
		return nullptr;
	}
	P->Damage = InDamage;
	P->bExplosive = bInExplosive;
	P->Color = InColor;
	P->Movement->InitialSpeed = Speed;
	P->Movement->MaxSpeed = Speed;
	if (Shooter)
	{
		P->Collision->IgnoreActorWhenMoving(Shooter, true);
	}
	P->FinishSpawning(Transform);

	JJAssets::SetupPart(P->Ball, JJAssets::Sphere(), InColor, FVector(Size), FVector::ZeroVector);
	P->Light->SetIntensityUnits(ELightUnits::Candelas);
	P->Light->SetIntensity(40.f);
	P->Light->SetAttenuationRadius(700.f);
	P->Light->SetLightColor(InColor);
	return P;
}

void AJJProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bDone)
	{
		return;
	}
	bDone = true;
	const FVector Where = GetActorLocation();
	AController* InstigatorController = GetInstigatorController();
	if (bExplosive)
	{
		if (OtherActor)
		{
			UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetActorForwardVector(), Hit, InstigatorController, this, UDamageType::StaticClass());
		}
		TArray<AActor*> Ignore;
		Ignore.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, 128.f, Where, 1000.f, UDamageType::StaticClass(), Ignore, this, InstigatorController, false, ECC_Visibility);
		AJJFlash::Spawn(GetWorld(), Where, FLinearColor(1.f, 0.45f, 0.1f), 2.2f, 2500.f, 0.6f);
	}
	else
	{
		if (OtherActor)
		{
			UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetActorForwardVector(), Hit, InstigatorController, this, UDamageType::StaticClass());
		}
		AJJFlash::Spawn(GetWorld(), Where, Color, 0.6f, 300.f, 0.3f);
	}
	Destroy();
}
