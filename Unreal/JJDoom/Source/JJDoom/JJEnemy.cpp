#include "JJEnemy.h"
#include "JJAssets.h"
#include "JJGameMode.h"
#include "JJCharacter.h"
#include "JJLevelBuilder.h"
#include "JJProjectile.h"
#include "JJFlash.h"
#include "JJPickup.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"

AJJEnemy::AJJEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	// Los disparos del jugador son trazas de "Visibility": la cápsula debe bloquearlas.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	auto MakePart = [this](const TCHAR* Name)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(GetCapsuleComponent());
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return Part;
	};
	Body = MakePart(TEXT("Body"));
	Head = MakePart(TEXT("Head"));
	Eyes = MakePart(TEXT("Eyes"));
	HornL = MakePart(TEXT("HornL"));
	HornR = MakePart(TEXT("HornR"));
	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(GetCapsuleComponent());
	Glow->SetCastShadows(false);
}

bool AJJEnemy::IsBoss() const
{
	return FJJEnemyDef::Get(Kind).bBoss;
}

void AJJEnemy::BeginPlay()
{
	Super::BeginPlay();
	const FJJEnemyDef& D = FJJEnemyDef::Get(Kind);
	Health = D.Health;
	Seed = FMath::FRand() * 10.f;
	Cooldown = FMath::FRandRange(0.5f, 1.5f);

	const float S = D.Scale;
	const float Radius = (Kind == EJJEnemyKind::Caco ? 85.f : Kind == EJJEnemyKind::Demon || Kind == EJJEnemyKind::Spectre ? 60.f
		: Kind == EJJEnemyKind::LostSoul ? 35.f : 40.f) * S;
	const float HalfHeight = (Kind == EJJEnemyKind::LostSoul ? 40.f : Kind == EJJEnemyKind::Caco ? 90.f : 90.f) * S;
	GetCapsuleComponent()->SetCapsuleSize(Radius, FMath::Max(Radius, HalfHeight));

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = D.Speed;
	Move->MaxFlySpeed = D.Speed;
	Move->BrakingDecelerationFlying = 3000.f;
	if (D.bFlying)
	{
		Move->GravityScale = 0.f;
		Move->SetMovementMode(MOVE_Flying);
	}
	BuildBody();
}

void AJJEnemy::BuildBody()
{
	using namespace JJAssets;
	const FJJEnemyDef& D = FJJEnemyDef::Get(Kind);
	const float S = D.Scale;
	HornL->SetVisibility(false);
	HornR->SetVisibility(false);
	Glow->SetVisibility(false);
	const FLinearColor Bone(0.78f, 0.72f, 0.58f);

	switch (Kind)
	{
	case EJJEnemyKind::Zombie:
	case EJJEnemyKind::Sergeant:
	case EJJEnemyKind::Imp:
	case EJJEnemyKind::Baron:
	{
		const FLinearColor Skin = Kind == EJJEnemyKind::Baron ? FLinearColor(0.62f, 0.4f, 0.32f)
			: Kind == EJJEnemyKind::Imp ? FLinearColor(0.26f, 0.12f, 0.05f) : FLinearColor(0.55f, 0.43f, 0.33f);
		const FLinearColor EyeColor = Kind == EJJEnemyKind::Imp ? FLinearColor(1.f, 0.7f, 0.f) : FLinearColor(1.f, 0.05f, 0.02f);
		SetupPart(Body, Cylinder(), D.Color, FVector(0.75f, 0.5f, 1.25f) * S, FVector(0.f, 0.f, -28.f * S));
		SetupPart(Head, Sphere(), Skin, FVector(0.42f) * S, FVector(0.f, 0.f, 58.f * S));
		SetupPart(Eyes, Cube(), EyeColor, FVector(0.05f, 0.28f, 0.05f) * S, FVector(19.f * S, 0.f, 62.f * S));
		if (Kind == EJJEnemyKind::Baron || Kind == EJJEnemyKind::Imp)
		{
			const bool bBaron = Kind == EJJEnemyKind::Baron;
			const FVector HornScale = FVector(0.1f, 0.1f, bBaron ? 0.4f : 0.25f) * S;
			const float HornY = (bBaron ? 16.f : 40.f) * S;
			const float HornZ = (bBaron ? 85.f : 30.f) * S;
			SetupPart(HornL, Cone(), Bone, HornScale, FVector(0.f, -HornY, HornZ), FRotator(0.f, 0.f, -30.f));
			SetupPart(HornR, Cone(), Bone, HornScale, FVector(0.f, HornY, HornZ), FRotator(0.f, 0.f, 30.f));
		}
		Glow->SetRelativeLocation(FVector(30.f * S, 0.f, 62.f * S));
		Glow->SetIntensityUnits(ELightUnits::Candelas);
		Glow->SetIntensity(2.f);
		Glow->SetAttenuationRadius(160.f * S);
		Glow->SetLightColor(EyeColor);
		Glow->SetVisibility(true);
		break;
	}
	case EJJEnemyKind::Demon:
	case EJJEnemyKind::Spectre:
		SetupPart(Body, Sphere(), D.Color, FVector(1.25f, 1.05f, 0.95f) * S, FVector(0.f, 0.f, -35.f * S));
		SetupPart(Head, Sphere(), D.Color * 1.1f, FVector(0.85f) * S, FVector(22.f * S, 0.f, 22.f * S));
		SetupPart(Eyes, Cube(), FLinearColor(1.f, 0.9f, 0.f), FVector(0.05f, 0.35f, 0.06f) * S, FVector(62.f * S, 0.f, 38.f * S));
		SetupPart(HornL, Cone(), Bone, FVector(0.12f, 0.12f, 0.3f) * S, FVector(18.f * S, -30.f * S, 62.f * S), FRotator(0.f, 0.f, -25.f));
		SetupPart(HornR, Cone(), Bone, FVector(0.12f, 0.12f, 0.3f) * S, FVector(18.f * S, 30.f * S, 62.f * S), FRotator(0.f, 0.f, 25.f));
		break;
	case EJJEnemyKind::LostSoul:
		SetupPart(Body, Sphere(), Bone, FVector(0.65f) * S, FVector::ZeroVector);
		SetupPart(Head, Cube(), FLinearColor(0.02f, 0.01f, 0.f), FVector(0.05f, 0.4f, 0.12f) * S, FVector(31.f * S, 0.f, 6.f * S));
		SetupPart(Eyes, Cube(), Bone, FVector(0.05f, 0.3f, 0.12f) * S, FVector(24.f * S, 0.f, -22.f * S));
		Glow->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
		Glow->SetIntensityUnits(ELightUnits::Candelas);
		Glow->SetIntensity(30.f);
		Glow->SetAttenuationRadius(700.f);
		Glow->SetLightColor(FLinearColor(1.f, 0.4f, 0.05f));
		Glow->SetVisibility(true);
		break;
	case EJJEnemyKind::Caco:
		SetupPart(Body, Sphere(), D.Color, FVector(1.7f) * S, FVector::ZeroVector);
		SetupPart(Eyes, Sphere(), FLinearColor(0.9f, 0.9f, 0.9f), FVector(0.45f) * S, FVector(66.f * S, 0.f, 18.f * S));
		SetupPart(Head, Sphere(), FLinearColor(0.05f, 0.7f, 0.05f), FVector(0.2f) * S, FVector(86.f * S, 0.f, 18.f * S));
		SetupPart(HornL, Cone(), Bone, FVector(0.15f, 0.15f, 0.35f) * S, FVector(0.f, -45.f * S, 80.f * S), FRotator(0.f, 0.f, -25.f));
		SetupPart(HornR, Cone(), Bone, FVector(0.15f, 0.15f, 0.35f) * S, FVector(0.f, 45.f * S, 80.f * S), FRotator(0.f, 0.f, 25.f));
		Glow->SetRelativeLocation(FVector(95.f * S, 0.f, -20.f * S));
		Glow->SetIntensityUnits(ELightUnits::Candelas);
		Glow->SetIntensity(4.f);
		Glow->SetAttenuationRadius(300.f);
		Glow->SetLightColor(D.ProjColor);
		Glow->SetVisibility(true);
		break;
	}
}

bool AJJEnemy::CanSee(const APawn* Player) const
{
	FHitResult Hit;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(JJEnemySight), false, this);
	const FVector From = GetActorLocation() + FVector(0.f, 0.f, 40.f);
	const FVector To = Player->GetPawnViewLocation();
	if (!GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, Query))
	{
		return true;
	}
	return Hit.GetActor() == Player;
}

void AJJEnemy::FaceTowards(const FVector& Where)
{
	const FVector To = Where - GetActorLocation();
	SetActorRotation(FRotator(0.f, To.Rotation().Yaw, 0.f));
}

void AJJEnemy::Wake()
{
	if (State == EJJEnemyState::Idle)
	{
		State = EJJEnemyState::Chase;
		Cooldown = FMath::FRandRange(0.4f, 1.4f);
	}
}

void AJJEnemy::StartAttack(bool bClose)
{
	State = EJJEnemyState::Attack;
	bFired = false;
	bMelee = bClose;
	StateTime = bClose ? 0.45f : 0.6f;
	FireAt = bClose ? 0.2f : 0.3f;
	Glow->SetIntensity(Glow->Intensity * 3.f);
}

void AJJEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AnimTime += DeltaSeconds;
	const FJJEnemyDef& D = FJJEnemyDef::Get(Kind);

	if (State == EJJEnemyState::Dead)
	{
		return;
	}
	if (State == EJJEnemyState::Dying)
	{
		StateTime += DeltaSeconds;
		const float A = FMath::Clamp(StateTime / 0.45f, 0.f, 1.f);
		const float FloorZ = 30.f;
		const FVector Pos(DeathStart.X, DeathStart.Y, FMath::Lerp(DeathStart.Z, FloorZ, A));
		SetActorLocationAndRotation(Pos, FRotator(0.f, DeathYaw, A * 85.f));
		if (A >= 1.f)
		{
			State = EJJEnemyState::Dead;
			if (Kind == EJJEnemyKind::LostSoul)
			{
				Destroy();
			}
		}
		return;
	}

	// El espectro parpadea: se ve a ratos, como una sombra.
	if (D.bGhost)
	{
		const bool bShow = FMath::FRand() > 0.4f;
		Body->SetVisibility(bShow);
		Head->SetVisibility(bShow || FMath::FRand() > 0.6f);
	}

	AJJGameMode* GM = AJJGameMode::Get(this);
	AJJCharacter* Player = Cast<AJJCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!GM || !Player || GM->State != EJJGameState::Playing)
	{
		return;
	}
	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	const float Dist = ToPlayer.Size2D();

	SightTimer -= DeltaSeconds;
	if (SightTimer <= 0.f)
	{
		SightTimer = 0.25f;
		bSeesPlayer = Dist < 9600.f && CanSee(Player);
	}

	switch (State)
	{
	case EJJEnemyState::Idle:
		if (bSeesPlayer && Dist < 7200.f)
		{
			Wake();
		}
		return;
	case EJJEnemyState::Pain:
		StateTime -= DeltaSeconds;
		if (StateTime <= 0.f)
		{
			State = EJJEnemyState::Chase;
		}
		return;
	case EJJEnemyState::Attack:
		FaceTowards(Player->GetActorLocation());
		StateTime -= DeltaSeconds;
		if (!bFired && StateTime <= FireAt)
		{
			bFired = true;
			DoAttack(Player, Dist);
		}
		if (StateTime <= 0.f)
		{
			State = EJJEnemyState::Chase;
			Cooldown = FMath::FRandRange(D.CooldownMin, D.CooldownMax);
			Glow->SetIntensity(Glow->Intensity / 3.f);
		}
		return;
	case EJJEnemyState::Charge:
		StateTime -= DeltaSeconds;
		GetCharacterMovement()->Velocity = ChargeDir * 2600.f;
		if ((Player->GetActorLocation() - GetActorLocation()).Size() < 160.f)
		{
			UGameplayStatics::ApplyDamage(Player, FMath::FRandRange(D.MeleeMin, D.MeleeMax), GetController(), this, UDamageType::StaticClass());
			GetCharacterMovement()->Velocity = FVector::ZeroVector;
			State = EJJEnemyState::Chase;
			Cooldown = FMath::FRandRange(D.CooldownMin, D.CooldownMax);
		}
		else if (StateTime <= 0.f || (StateTime < 1.2f && GetVelocity().Size() < 500.f))
		{
			State = EJJEnemyState::Chase;
			Cooldown = FMath::FRandRange(D.CooldownMin, D.CooldownMax);
		}
		return;
	default:
		break;
	}

	if (Player->IsDead())
	{
		return;
	}

	// Decidir si atacar.
	Cooldown -= DeltaSeconds;
	if (Cooldown <= 0.f && bSeesPlayer)
	{
		if (D.Attack == EJJAttack::Charge)
		{
			if (Dist < D.Range)
			{
				State = EJJEnemyState::Charge;
				StateTime = 1.4f;
				ChargeDir = (Player->GetPawnViewLocation() - GetActorLocation()).GetSafeNormal();
				FaceTowards(Player->GetActorLocation());
				return;
			}
		}
		else
		{
			const bool bClose = Dist < 220.f;
			if (bClose || (D.Attack != EJJAttack::Melee && Dist < D.Range && FMath::FRand() < 0.6f))
			{
				StartAttack(bClose);
				return;
			}
		}
		Cooldown = 0.3f;
	}

	// Moverse: directo (zigzagueando) si ve al jugador; si no, siguiendo el camino del mapa.
	FVector Dir;
	if (bSeesPlayer)
	{
		const float Yaw = ToPlayer.Rotation().Yaw + FMath::Sin(AnimTime * 1.3f + Seed) * 30.f;
		Dir = FRotator(0.f, Yaw, 0.f).Vector();
	}
	else
	{
		FVector Target;
		AJJLevelBuilder* Builder = GM->GetBuilder();
		if (!Builder || !Builder->NextStepTowardsPlayer(GetActorLocation(), Target))
		{
			return;
		}
		Dir = (Target - GetActorLocation()).GetSafeNormal2D();
	}
	if (D.bFlying)
	{
		Dir.Z = FMath::Clamp((220.f - GetActorLocation().Z) / 200.f, -1.f, 1.f);
	}
	if (!bSeesPlayer || Dist > 180.f)
	{
		AddMovementInput(Dir, 1.f);
	}
}

void AJJEnemy::DoAttack(APawn* Player, float Distance)
{
	const FJJEnemyDef& D = FJJEnemyDef::Get(Kind);
	AController* MyController = GetController();
	if (bMelee)
	{
		if (Distance < 260.f)
		{
			UGameplayStatics::ApplyDamage(Player, FMath::FRandRange(D.MeleeMin, D.MeleeMax), MyController, this, UDamageType::StaticClass());
		}
		return;
	}
	if (!CanSee(Player))
	{
		return;
	}
	const FVector Muzzle = GetActorLocation() + GetActorForwardVector() * (70.f * D.Scale) + FVector(0.f, 0.f, 25.f * D.Scale);
	if (D.Attack == EJJAttack::Hitscan || D.Attack == EJJAttack::Shotgun)
	{
		AJJFlash::Spawn(GetWorld(), Muzzle, FLinearColor(1.f, 0.8f, 0.3f), 0.15f, 80.f, 0.08f);
		const int32 Shots = D.Attack == EJJAttack::Shotgun ? 3 : 1;
		const float Chance = FMath::Clamp(1.15f - Distance / 4800.f, 0.25f, 0.85f);
		float Damage = 0.f;
		for (int32 I = 0; I < Shots; ++I)
		{
			if (FMath::FRand() < Chance)
			{
				Damage += FMath::FRandRange(D.ShotMin, D.ShotMax);
			}
		}
		if (Damage > 0.f)
		{
			UGameplayStatics::ApplyDamage(Player, Damage, MyController, this, UDamageType::StaticClass());
		}
	}
	else if (D.Attack == EJJAttack::Projectile)
	{
		const FVector Aim = (Player->GetPawnViewLocation() - FVector(0.f, 0.f, 20.f) - Muzzle).GetSafeNormal();
		TArray<float> Offsets = { 0.f };
		// El Barón herido lanza tres bolas en abanico.
		if (D.bBoss && Health < D.Health * 0.5f)
		{
			Offsets.Add(-8.f);
			Offsets.Add(8.f);
		}
		for (const float Offset : Offsets)
		{
			const FVector Dir = Aim.RotateAngleAxis(Offset + FMath::FRandRange(-2.f, 2.f), FVector::UpVector);
			AJJProjectile::Fire(GetWorld(), this, Muzzle, Dir, FMath::FRandRange(D.ShotMin, D.ShotMax), D.ProjSpeed, D.ProjColor, false,
				Kind == EJJEnemyKind::Baron ? 0.45f : 0.35f);
		}
	}
}

float AJJEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsAlive() || DamageAmount <= 0.f)
	{
		return 0.f;
	}
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health -= DamageAmount;
	if (Health <= 0.f)
	{
		Die();
		return DamageAmount;
	}
	Wake();
	if (FMath::FRand() < FJJEnemyDef::Get(Kind).PainChance)
	{
		State = EJJEnemyState::Pain;
		StateTime = 0.25f;
	}
	return DamageAmount;
}

void AJJEnemy::Die()
{
	State = EJJEnemyState::Dying;
	StateTime = 0.f;
	DeathStart = GetActorLocation();
	DeathYaw = GetActorRotation().Yaw;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	Body->SetVisibility(true);
	Head->SetVisibility(true);
	AJJFlash::Spawn(GetWorld(), GetActorLocation(), FLinearColor(0.35f, 0.f, 0.f), 0.5f, 0.f, 0.35f);

	const FVector Drop(GetActorLocation().X, GetActorLocation().Y, 40.f);
	if (Kind == EJJEnemyKind::Zombie)
	{
		AJJPickup::SpawnPickup(GetWorld(), EJJPickup::Clip, Drop, 5, false);
	}
	else if (Kind == EJJEnemyKind::Sergeant)
	{
		AJJPickup::SpawnPickup(GetWorld(), EJJPickup::Shells, Drop, 4, false);
	}
	if (AJJGameMode* GM = AJJGameMode::Get(this))
	{
		GM->AddKill(this);
	}
}
