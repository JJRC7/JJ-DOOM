#include "JJCharacter.h"
#include "JJAssets.h"
#include "JJGameMode.h"
#include "JJDoor.h"
#include "JJExitSwitch.h"
#include "JJEnemy.h"
#include "JJProjectile.h"
#include "JJFlash.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"

namespace
{
	const float JJWalkSpeed = 1100.f;
	const float JJSprintSpeed = 1700.f;
}

AJJCharacter::AJJCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = true;
	Camera->SetFieldOfView(95.f);

	auto MakePart = [this](const TCHAR* Name)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(Camera);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCastShadow(false);
		return Part;
	};
	GunBody = MakePart(TEXT("GunBody"));
	GunBarrel = MakePart(TEXT("GunBarrel"));
	GunHand = MakePart(TEXT("GunHand"));

	MuzzleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleLight"));
	MuzzleLight->SetupAttachment(Camera);

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(Camera);
	Flashlight->SetRelativeLocation(FVector(10.f, 15.f, -15.f));

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = JJWalkSpeed;
	Move->MaxAcceleration = 6000.f;
	Move->BrakingDecelerationWalking = 6000.f;
	Move->GroundFriction = 10.f;
	Move->AirControl = 0.5f;
}

void AJJCharacter::BeginPlay()
{
	Super::BeginPlay();
	MuzzleLight->SetIntensityUnits(ELightUnits::Candelas);
	MuzzleLight->SetIntensity(120.f);
	MuzzleLight->SetAttenuationRadius(1500.f);
	MuzzleLight->SetLightColor(FLinearColor(1.f, 0.75f, 0.35f));
	MuzzleLight->SetVisibility(false);

	Flashlight->SetIntensityUnits(ELightUnits::Candelas);
	Flashlight->SetIntensity(400.f);
	Flashlight->SetAttenuationRadius(3500.f);
	Flashlight->SetInnerConeAngle(12.f);
	Flashlight->SetOuterConeAngle(28.f);
	Flashlight->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));

	UpdateWeaponModel();
	if (AJJGameMode* GM = AJJGameMode::Get(this))
	{
		GM->PlacePlayer(this);
	}
}

void AJJCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (AJJGameMode* GM = AJJGameMode::Get(this))
	{
		GM->PlacePlayer(this);
	}
}

void AJJCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);
	Input->BindAxis(TEXT("MoveForward"), this, &AJJCharacter::MoveForward);
	Input->BindAxis(TEXT("MoveRight"), this, &AJJCharacter::MoveRight);
	Input->BindAxis(TEXT("Turn"), this, &AJJCharacter::Turn);
	Input->BindAxis(TEXT("LookUp"), this, &AJJCharacter::LookUp);
	Input->BindAxis(TEXT("TurnRate"), this, &AJJCharacter::TurnRate);
	Input->BindAxis(TEXT("LookUpRate"), this, &AJJCharacter::LookUpRate);
	Input->BindAction(TEXT("Fire"), IE_Pressed, this, &AJJCharacter::StartFire);
	Input->BindAction(TEXT("Fire"), IE_Released, this, &AJJCharacter::StopFire);
	Input->BindAction(TEXT("Use"), IE_Pressed, this, &AJJCharacter::Use);
	Input->BindAction(TEXT("Sprint"), IE_Pressed, this, &AJJCharacter::SprintOn);
	Input->BindAction(TEXT("Sprint"), IE_Released, this, &AJJCharacter::SprintOff);
	Input->BindAction(TEXT("Flashlight"), IE_Pressed, this, &AJJCharacter::ToggleFlashlight);
	Input->BindAction(TEXT("Continue"), IE_Pressed, this, &AJJCharacter::Continue);
	Input->BindAction(TEXT("Weapon1"), IE_Pressed, this, &AJJCharacter::Weapon1);
	Input->BindAction(TEXT("Weapon2"), IE_Pressed, this, &AJJCharacter::Weapon2);
	Input->BindAction(TEXT("Weapon3"), IE_Pressed, this, &AJJCharacter::Weapon3);
	Input->BindAction(TEXT("Weapon4"), IE_Pressed, this, &AJJCharacter::Weapon4);
	Input->BindAction(TEXT("NextWeapon"), IE_Pressed, this, &AJJCharacter::NextWeapon);
	Input->BindAction(TEXT("PrevWeapon"), IE_Pressed, this, &AJJCharacter::PrevWeapon);
}

bool AJJCharacter::CanAct() const
{
	const AJJGameMode* GM = AJJGameMode::Get(this);
	return !bDead && GM && GM->State == EJJGameState::Playing;
}

void AJJCharacter::MoveForward(float Value)
{
	if (Value != 0.f && CanAct())
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AJJCharacter::MoveRight(float Value)
{
	if (Value != 0.f && CanAct())
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AJJCharacter::Turn(float Value)
{
	if (!bDead) AddControllerYawInput(Value);
}

void AJJCharacter::LookUp(float Value)
{
	if (!bDead) AddControllerPitchInput(Value);
}

void AJJCharacter::TurnRate(float Value)
{
	if (!bDead && Value != 0.f) AddControllerYawInput(Value * 60.f * GetWorld()->GetDeltaSeconds());
}

void AJJCharacter::LookUpRate(float Value)
{
	if (!bDead && Value != 0.f) AddControllerPitchInput(Value * 45.f * GetWorld()->GetDeltaSeconds());
}

void AJJCharacter::StartFire()
{
	if (!CanAct())
	{
		Continue();
		return;
	}
	bFiring = true;
}

void AJJCharacter::StopFire()
{
	bFiring = false;
}

void AJJCharacter::SprintOn()
{
	GetCharacterMovement()->MaxWalkSpeed = JJSprintSpeed;
}

void AJJCharacter::SprintOff()
{
	GetCharacterMovement()->MaxWalkSpeed = JJWalkSpeed;
}

void AJJCharacter::ToggleFlashlight()
{
	Flashlight->SetVisibility(!Flashlight->IsVisible());
}

void AJJCharacter::Continue()
{
	if (AJJGameMode* GM = AJJGameMode::Get(this))
	{
		GM->OnContinuePressed();
	}
}

void AJJCharacter::Weapon1() { SelectWeapon(EJJWeapon::Pistol); }
void AJJCharacter::Weapon2() { SelectWeapon(EJJWeapon::Shotgun); }
void AJJCharacter::Weapon3() { SelectWeapon(EJJWeapon::Chaingun); }
void AJJCharacter::Weapon4() { SelectWeapon(EJJWeapon::Rocket); }
void AJJCharacter::NextWeapon() { CycleWeapon(1); }
void AJJCharacter::PrevWeapon() { CycleWeapon(-1); }

void AJJCharacter::CycleWeapon(int32 Step)
{
	int32 Index = static_cast<int32>(Inv.Current);
	for (int32 I = 0; I < 4; ++I)
	{
		Index = (Index + Step + 4) % 4;
		if (Inv.bHas[Index])
		{
			SelectWeapon(static_cast<EJJWeapon>(Index));
			return;
		}
	}
}

bool AJJCharacter::SelectWeapon(EJJWeapon Weapon)
{
	const int32 Index = static_cast<int32>(Weapon);
	if (!Inv.bHas[Index] || Inv.Current == Weapon)
	{
		return false;
	}
	Inv.Current = Weapon;
	SwitchTime = 1.f;
	UpdateWeaponModel();
	return true;
}

void AJJCharacter::AutoSwitch()
{
	const EJJWeapon Order[] = { EJJWeapon::Chaingun, EJJWeapon::Shotgun, EJJWeapon::Pistol, EJJWeapon::Rocket };
	for (const EJJWeapon Weapon : Order)
	{
		const int32 Index = static_cast<int32>(Weapon);
		if (Inv.bHas[Index] && Inv.Ammo[static_cast<int32>(FJJWeaponDef::Get(Weapon).Ammo)] > 0)
		{
			SelectWeapon(Weapon);
			return;
		}
	}
}

void AJJCharacter::UpdateWeaponModel()
{
	using namespace JJAssets;
	const FLinearColor Skin(0.55f, 0.36f, 0.22f);
	const FLinearColor Steel(0.05f, 0.05f, 0.055f);
	const FRotator Forward(90.f, 0.f, 0.f); // el cilindro del motor es vertical: se tumba hacia delante
	switch (Inv.Current)
	{
	case EJJWeapon::Pistol:
		BodyBase = FVector(36.f, 14.f, -18.f);
		BarrelBase = FVector(50.f, 14.f, -13.f);
		HandBase = FVector(30.f, 14.f, -23.f);
		SetupPart(GunBody, Cube(), Steel, FVector(0.3f, 0.07f, 0.12f), BodyBase);
		SetupPart(GunBarrel, Cylinder(), Steel, FVector(0.04f, 0.04f, 0.22f), BarrelBase, Forward);
		SetupPart(GunHand, Sphere(), Skin, FVector(0.11f, 0.09f, 0.1f), HandBase);
		break;
	case EJJWeapon::Shotgun:
		BodyBase = FVector(36.f, 12.f, -20.f);
		BarrelBase = FVector(62.f, 12.f, -14.f);
		HandBase = FVector(52.f, 12.f, -22.f);
		SetupPart(GunBody, Cube(), FLinearColor(0.2f, 0.09f, 0.03f), FVector(0.35f, 0.07f, 0.09f), BodyBase);
		SetupPart(GunBarrel, Cylinder(), Steel, FVector(0.05f, 0.05f, 0.7f), BarrelBase, Forward);
		SetupPart(GunHand, Sphere(), Skin, FVector(0.12f, 0.1f, 0.1f), HandBase);
		break;
	case EJJWeapon::Chaingun:
		BodyBase = FVector(34.f, 14.f, -20.f);
		BarrelBase = FVector(58.f, 14.f, -16.f);
		HandBase = FVector(28.f, 14.f, -27.f);
		SetupPart(GunBody, Cube(), FLinearColor(0.08f, 0.08f, 0.08f), FVector(0.35f, 0.14f, 0.14f), BodyBase);
		SetupPart(GunBarrel, Cylinder(), Steel, FVector(0.13f, 0.13f, 0.6f), BarrelBase, Forward);
		SetupPart(GunHand, Sphere(), Skin, FVector(0.12f, 0.1f, 0.1f), HandBase);
		break;
	case EJJWeapon::Rocket:
		BodyBase = FVector(30.f, 16.f, -24.f);
		BarrelBase = FVector(50.f, 16.f, -15.f);
		HandBase = FVector(30.f, 16.f, -30.f);
		SetupPart(GunBody, Cube(), FLinearColor(0.6f, 0.45f, 0.02f), FVector(0.2f, 0.12f, 0.14f), BodyBase);
		SetupPart(GunBarrel, Cylinder(), FLinearColor(0.12f, 0.16f, 0.08f), FVector(0.16f, 0.16f, 0.9f), BarrelBase, Forward);
		SetupPart(GunHand, Sphere(), Skin, FVector(0.12f, 0.1f, 0.1f), HandBase);
		break;
	}
	for (UStaticMeshComponent* Part : { GunBody.Get(), GunBarrel.Get(), GunHand.Get() })
	{
		Part->SetCastShadow(false);
	}
	MuzzleLight->SetRelativeLocation(BarrelBase + FVector(45.f, 0.f, 0.f));
}

void AJJCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DamageFlash = FMath::Max(0.f, DamageFlash - DeltaSeconds * 1.5f);
	PickupFlash = FMath::Max(0.f, PickupFlash - DeltaSeconds * 2.f);

	if (bDead)
	{
		DeadTime += DeltaSeconds;
		Camera->SetRelativeLocation(FVector(0.f, 0.f, FMath::Lerp(64.f, -60.f, FMath::Min(1.f, DeadTime))));
		return;
	}

	Cooldown -= DeltaSeconds;
	MuzzleTime -= DeltaSeconds;
	SwitchTime = FMath::Max(0.f, SwitchTime - DeltaSeconds * 4.f);
	MuzzleLight->SetVisibility(MuzzleTime > 0.f);
	if (bFiring && CanAct())
	{
		FireWeapon();
	}

	// Balanceo del arma al caminar, retroceso al disparar y bajada al cambiar de arma.
	const float Speed = GetVelocity().Size2D();
	BobTime += DeltaSeconds * Speed / 110.f;
	const float Amount = FMath::Clamp(Speed / JJWalkSpeed, 0.f, 1.f);
	const FVector Offset(MuzzleTime > 0.f ? -3.f : 0.f, FMath::Cos(BobTime) * 1.5f * Amount,
		-FMath::Abs(FMath::Sin(BobTime)) * 1.5f * Amount - SwitchTime * 25.f);
	GunBody->SetRelativeLocation(BodyBase + Offset);
	GunBarrel->SetRelativeLocation(BarrelBase + Offset);
	GunHand->SetRelativeLocation(HandBase + Offset);
}

void AJJCharacter::FireWeapon()
{
	if (Cooldown > 0.f || SwitchTime > 0.f)
	{
		return;
	}
	const FJJWeaponDef& W = FJJWeaponDef::Get(Inv.Current);
	int32& Ammo = Inv.Ammo[static_cast<int32>(W.Ammo)];
	if (Ammo <= 0)
	{
		AutoSwitch();
		Cooldown = 0.35f;
		return;
	}
	Ammo--;
	Cooldown = W.Cooldown;
	MuzzleTime = 0.05f;

	const FVector Eye = Camera->GetComponentLocation();
	const FVector Aim = GetControlRotation().Vector();
	if (W.bRocket)
	{
		AJJProjectile::Fire(GetWorld(), this, Eye + Aim * 70.f - FVector(0.f, 0.f, 15.f), Aim, FMath::FRandRange(W.DamageMin, W.DamageMax),
			2600.f, FLinearColor(1.f, 0.5f, 0.1f), true, 0.3f);
	}
	else
	{
		FCollisionQueryParams Query(SCENE_QUERY_STAT(JJShot), false, this);
		for (int32 I = 0; I < W.Pellets; ++I)
		{
			const FVector Dir = FMath::VRandCone(Aim, W.Spread);
			FHitResult Hit;
			if (!GetWorld()->LineTraceSingleByChannel(Hit, Eye, Eye + Dir * 12000.f, ECC_Visibility, Query))
			{
				continue;
			}
			AActor* Target = Hit.GetActor();
			const bool bFlesh = Cast<AJJEnemy>(Target) != nullptr;
			if (Target)
			{
				UGameplayStatics::ApplyPointDamage(Target, FMath::FRandRange(W.DamageMin, W.DamageMax), Dir, Hit, GetController(), this, UDamageType::StaticClass());
			}
			AJJFlash::Spawn(GetWorld(), Hit.ImpactPoint + Hit.ImpactNormal * 4.f,
				bFlesh ? FLinearColor(0.4f, 0.f, 0.f) : FLinearColor(0.55f, 0.5f, 0.42f), 0.08f, bFlesh ? 0.f : 10.f, 0.25f);
		}
	}
	AddControllerPitchInput(W.bRocket ? -0.6f : -0.12f);
	if (AJJGameMode* GM = AJJGameMode::Get(this))
	{
		GM->AlertEnemies(GetActorLocation(), 5200.f);
	}
}

void AJJCharacter::Use()
{
	if (!CanAct())
	{
		return;
	}
	const FVector Eye = Camera->GetComponentLocation();
	FHitResult Hit;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(JJUse), false, this);
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Eye, Eye + GetControlRotation().Vector() * 300.f, ECC_Visibility, Query))
	{
		return;
	}
	if (AJJDoor* Door = Cast<AJJDoor>(Hit.GetActor()))
	{
		Door->TryOpen(this);
	}
	else if (AJJExitSwitch* Exit = Cast<AJJExitSwitch>(Hit.GetActor()))
	{
		Exit->Activate();
	}
}

float AJJCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const AJJGameMode* GM = AJJGameMode::Get(this);
	if (bDead || DamageAmount <= 0.f || !GM || GM->State != EJJGameState::Playing)
	{
		return 0.f;
	}
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	// La armadura absorbe un tercio del daño.
	const float Absorbed = FMath::Min(Inv.Armor, FMath::FloorToFloat(DamageAmount / 3.f));
	Inv.Armor -= Absorbed;
	const float Taken = DamageAmount - Absorbed;
	Inv.Health -= Taken;
	DamageFlash = FMath::Min(0.8f, DamageFlash + Taken / 40.f + 0.15f);
	if (Inv.Health <= 0.f)
	{
		Inv.Health = 0.f;
		bDead = true;
		bFiring = false;
		DeadTime = 0.f;
		MuzzleLight->SetVisibility(false);
		if (AJJGameMode* Mode = AJJGameMode::Get(this))
		{
			Mode->OnPlayerDied();
		}
	}
	return Taken;
}

void AJJCharacter::Revive()
{
	bDead = false;
	bFiring = false;
	DeadTime = 0.f;
	DamageFlash = 0.f;
	bRedKey = false;
	bBlueKey = false;
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	UpdateWeaponModel();
}

bool AJJCharacter::TryPickup(EJJPickup Type, int32 Amount, bool bCounted)
{
	if (bDead)
	{
		return false;
	}
	FString Message;
	int32* Ammo = Inv.Ammo;
	switch (Type)
	{
	case EJJPickup::Medkit:
		if (Inv.Health >= 100.f) return false;
		Inv.Health = FMath::Min(100.f, Inv.Health + 25.f);
		Message = TEXT("Recogiste un botiquín");
		break;
	case EJJPickup::Stim:
		if (Inv.Health >= 100.f) return false;
		Inv.Health = FMath::Min(100.f, Inv.Health + 10.f);
		Message = TEXT("Recogiste un estimulante");
		break;
	case EJJPickup::Soul:
		Inv.Health = FMath::Min(200.f, Inv.Health + 100.f);
		Message = TEXT("¡ESFERA DE ALMAS!");
		break;
	case EJJPickup::Clip:
		if (Ammo[0] >= 200) return false;
		Ammo[0] = FMath::Min(200, Ammo[0] + (Amount > 0 ? Amount : 10));
		Message = TEXT("Recogiste un cargador");
		break;
	case EJJPickup::Shells:
		if (Ammo[1] >= 50) return false;
		Ammo[1] = FMath::Min(50, Ammo[1] + (Amount > 0 ? Amount : 8));
		Message = FString::Printf(TEXT("Recogiste %d cartuchos"), Amount > 0 ? Amount : 8);
		break;
	case EJJPickup::Rockets:
		if (Ammo[2] >= 50) return false;
		Ammo[2] = FMath::Min(50, Ammo[2] + 5);
		Message = TEXT("Recogiste 5 cohetes");
		break;
	case EJJPickup::Armor:
		if (Inv.Armor >= 100.f) return false;
		Inv.Armor = 100.f;
		Message = TEXT("Recogiste la armadura");
		break;
	case EJJPickup::RedKey:
		bRedKey = true;
		Message = TEXT("Recogiste la LLAVE ROJA");
		break;
	case EJJPickup::BlueKey:
		bBlueKey = true;
		Message = TEXT("Recogiste la LLAVE AZUL");
		break;
	case EJJPickup::Shotgun:
	case EJJPickup::Chaingun:
	case EJJPickup::RocketLauncher:
	{
		const EJJWeapon Weapon = Type == EJJPickup::Shotgun ? EJJWeapon::Shotgun : Type == EJJPickup::Chaingun ? EJJWeapon::Chaingun : EJJWeapon::Rocket;
		const int32 Index = static_cast<int32>(Weapon);
		const bool bNew = !Inv.bHas[Index];
		Inv.bHas[Index] = true;
		if (Weapon == EJJWeapon::Shotgun) Ammo[1] = FMath::Min(50, Ammo[1] + 8);
		else if (Weapon == EJJWeapon::Chaingun) Ammo[0] = FMath::Min(200, Ammo[0] + 20);
		else Ammo[2] = FMath::Min(50, Ammo[2] + 4);
		Message = Weapon == EJJWeapon::Shotgun ? TEXT("¡Conseguiste la ESCOPETA!")
			: Weapon == EJJWeapon::Chaingun ? TEXT("¡Conseguiste la AMETRALLADORA!") : TEXT("¡Conseguiste el LANZACOHETES!");
		if (bNew)
		{
			SelectWeapon(Weapon);
		}
		break;
	}
	}
	PickupFlash = 0.4f;
	if (AJJGameMode* GM = AJJGameMode::Get(this))
	{
		GM->ShowMessage(Message);
		if (bCounted)
		{
			GM->AddItem();
		}
	}
	return true;
}
