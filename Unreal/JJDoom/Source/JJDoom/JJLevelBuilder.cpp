#include "JJLevelBuilder.h"
#include "JJLevels.h"
#include "JJAssets.h"
#include "JJDoor.h"
#include "JJExitSwitch.h"
#include "JJEnemy.h"
#include "JJPickup.h"
#include "JJBarrel.h"
#include "JJProjectile.h"
#include "JJFlash.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "Engine/World.h"

namespace
{
	int32 JJWallType(TCHAR C)
	{
		switch (C)
		{
		case '#': return 1;
		case 'M': return 2;
		case 'S': return 3;
		case 'F': return 4;
		case 'X': return 5;
		case 'D': return 6;
		case 'R': return 7;
		case 'U': return 9;
		case 'T': return 10;
		case 'W': return 11;
		case 'G': return 12;
		default: return 0;
		}
	}

	bool JJIsDoor(int32 Type) { return Type == 6 || Type == 7 || Type == 9; }

	FLinearColor JJWallColor(int32 Type)
	{
		switch (Type)
		{
		case 1: return FLinearColor(0.30f, 0.09f, 0.05f);   // ladrillo
		case 2: return FLinearColor(0.24f, 0.26f, 0.29f);   // metal
		case 3: return FLinearColor(0.20f, 0.21f, 0.17f);   // piedra
		case 4: return FLinearColor(0.35f, 0.03f, 0.03f);   // carne
		case 10: return FLinearColor(0.10f, 0.12f, 0.14f);  // panel técnico
		case 11: return FLinearColor(0.25f, 0.13f, 0.05f);  // madera
		case 12: return FLinearColor(0.06f, 0.20f, 0.14f);  // mármol verde
		default: return FLinearColor(0.2f, 0.2f, 0.2f);
		}
	}

	bool JJEnemyFromChar(TCHAR C, EJJEnemyKind& Out)
	{
		switch (C)
		{
		case 'z': Out = EJJEnemyKind::Zombie; return true;
		case 's': Out = EJJEnemyKind::Sergeant; return true;
		case 'i': Out = EJJEnemyKind::Imp; return true;
		case 'd': Out = EJJEnemyKind::Demon; return true;
		case 'e': Out = EJJEnemyKind::Spectre; return true;
		case 'l': Out = EJJEnemyKind::LostSoul; return true;
		case 'C': Out = EJJEnemyKind::Caco; return true;
		case 'B': Out = EJJEnemyKind::Baron; return true;
		default: return false;
		}
	}

	bool JJPickupFromChar(TCHAR C, EJJPickup& Out)
	{
		switch (C)
		{
		case 'h': Out = EJJPickup::Medkit; return true;
		case 't': Out = EJJPickup::Stim; return true;
		case 'v': Out = EJJPickup::Soul; return true;
		case 'a': Out = EJJPickup::Clip; return true;
		case 'b': Out = EJJPickup::Shells; return true;
		case 'q': Out = EJJPickup::Rockets; return true;
		case 'r': Out = EJJPickup::Armor; return true;
		case 'g': Out = EJJPickup::Shotgun; return true;
		case 'c': Out = EJJPickup::Chaingun; return true;
		case 'n': Out = EJJPickup::RocketLauncher; return true;
		case 'k': Out = EJJPickup::RedKey; return true;
		case 'u': Out = EJJPickup::BlueKey; return true;
		default: return false;
		}
	}

	template <class T>
	void JJDestroyAll(UWorld* World)
	{
		for (TActorIterator<T> It(World); It; ++It)
		{
			It->Destroy();
		}
	}
}

AJJLevelBuilder::AJJLevelBuilder()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->SetupAttachment(Root);
	Sun->SetMobility(EComponentMobility::Movable);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->bRealTimeCapture = true;

	Fog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Fog"));
	Fog->SetupAttachment(Root);

	Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Atmosphere"));
	Atmosphere->SetupAttachment(Root);
}

FIntPoint AJJLevelBuilder::WorldToCell(const FVector& Where) const
{
	return FIntPoint(FMath::FloorToInt(Where.X / Cell), FMath::FloorToInt(Where.Y / Cell));
}

FVector AJJLevelBuilder::CellCenter(int32 X, int32 Y, float Z) const
{
	return FVector(X * Cell + Cell * 0.5f, Y * Cell + Cell * 0.5f, Z);
}

bool AJJLevelBuilder::IsSolidCell(int32 X, int32 Y) const
{
	if (X < 0 || Y < 0 || X >= W || Y >= H)
	{
		return true;
	}
	const int32 I = Y * W + X;
	if (Grid[I] == 0)
	{
		return false;
	}
	if (const TObjectPtr<AJJDoor>* Door = Doors.Find(I))
	{
		return !(*Door && (*Door)->IsPassable());
	}
	return true;
}

UInstancedStaticMeshComponent* AJJLevelBuilder::GetISM(int32 Key, const FLinearColor& Color, UMaterialInterface* Override, bool bCollision)
{
	if (TObjectPtr<UInstancedStaticMeshComponent>* Found = ISMs.Find(Key))
	{
		return *Found;
	}
	UInstancedStaticMeshComponent* ISM = NewObject<UInstancedStaticMeshComponent>(this);
	ISM->SetStaticMesh(JJAssets::Cube());
	UMaterialInterface* Material = Override ? Override : static_cast<UMaterialInterface*>(JJAssets::ColorMaterial(ISM, Color));
	ISM->SetMaterial(0, Material);
	if (bCollision)
	{
		ISM->SetCollisionProfileName(TEXT("BlockAll"));
	}
	else
	{
		ISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	ISM->SetupAttachment(Root);
	ISM->RegisterComponent();
	ISMs.Add(Key, ISM);
	return ISM;
}

void AJJLevelBuilder::AddLamp(int32 X, int32 Y, const FLinearColor& Color)
{
	UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
	Light->SetupAttachment(Root);
	Light->SetRelativeLocation(CellCenter(X, Y, WallHeight - 60.f));
	Light->SetIntensityUnits(ELightUnits::Candelas);
	Light->SetIntensity(LampCandelas);
	Light->SetAttenuationRadius(1800.f);
	Light->SetLightColor(Color);
	Light->SetCastShadows(true);
	Light->RegisterComponent();
	Lamps.Add(Light);

	// La lámpara visible en el techo.
	GetISM(102, FLinearColor(1.f, 0.95f, 0.8f), nullptr, false)
		->AddInstance(FTransform(FRotator::ZeroRotator, CellCenter(X, Y, WallHeight - 4.f), FVector(0.7f, 0.7f, 0.08f)));
}

void AJJLevelBuilder::Clear()
{
	UWorld* World = GetWorld();
	JJDestroyAll<AJJEnemy>(World);
	JJDestroyAll<AJJPickup>(World);
	JJDestroyAll<AJJDoor>(World);
	JJDestroyAll<AJJExitSwitch>(World);
	JJDestroyAll<AJJBarrel>(World);
	JJDestroyAll<AJJProjectile>(World);
	JJDestroyAll<AJJFlash>(World);
	for (TPair<int32, TObjectPtr<UInstancedStaticMeshComponent>>& Pair : ISMs)
	{
		if (Pair.Value)
		{
			Pair.Value->DestroyComponent();
		}
	}
	ISMs.Empty();
	for (TObjectPtr<UActorComponent>& Lamp : Lamps)
	{
		if (Lamp)
		{
			Lamp->DestroyComponent();
		}
	}
	Lamps.Empty();
	Doors.Empty();
	Grid.Empty();
	Flow.Empty();
	W = H = 0;
}

void AJJLevelBuilder::Build(int32 LevelIndex)
{
	Clear();
	const TArray<FJJLevelData>& Levels = JJLevels::Get();
	const FJJLevelData& L = Levels[FMath::Clamp(LevelIndex, 0, Levels.Num() - 1)];
	H = L.Map.Num();
	W = L.Map[0].Len();
	Grid.Init(0, W * H);
	Flow.Init(-1, W * H);
	TArray<bool> Outdoor;
	Outdoor.Init(false, W * H);
	for (const FIntRect& R : L.Sky)
	{
		for (int32 Y = R.Min.Y; Y <= R.Max.Y; ++Y)
		{
			for (int32 X = R.Min.X; X <= R.Max.X; ++X)
			{
				Outdoor[Y * W + X] = true;
			}
		}
	}
	TotalEnemies = 0;
	TotalItems = 0;
	UWorld* World = GetWorld();
	auto CharAt = [&L, this](int32 X, int32 Y) -> TCHAR
	{
		return (X < 0 || Y < 0 || X >= W || Y >= H) ? TCHAR('#') : L.Map[Y][X];
	};
	const FVector TileScale(Cell / 100.f, Cell / 100.f, 0.2f);

	for (int32 Y = 0; Y < H; ++Y)
	{
		for (int32 X = 0; X < W; ++X)
		{
			const TCHAR C = CharAt(X, Y);
			const int32 Type = JJWallType(C);
			const int32 I = Y * W + X;
			if (Type != 0)
			{
				Grid[I] = static_cast<uint8>(Type);
				if (JJIsDoor(Type))
				{
					const bool bAlongX = JJWallType(CharAt(X - 1, Y)) != 0 && JJWallType(CharAt(X + 1, Y)) != 0;
					AJJDoor* Door = World->SpawnActor<AJJDoor>(AJJDoor::StaticClass(), CellCenter(X, Y), FRotator::ZeroRotator);
					if (Door)
					{
						Door->Setup(Type == 7 ? EJJKey::Red : Type == 9 ? EJJKey::Blue : EJJKey::None, bAlongX);
						Doors.Add(I, Door);
					}
					// Suelo y techo bajo la puerta (se esconde en el techo al abrirse).
					GetISM(100, L.FloorColor, FloorMaterial, true)->AddInstance(FTransform(FRotator::ZeroRotator, CellCenter(X, Y, -10.f), TileScale));
					GetISM(101, L.CeilingColor, CeilingMaterial, true)->AddInstance(FTransform(FRotator::ZeroRotator, CellCenter(X, Y, WallHeight + 10.f), TileScale));
				}
				else if (Type == 5)
				{
					World->SpawnActor<AJJExitSwitch>(AJJExitSwitch::StaticClass(), CellCenter(X, Y), FRotator::ZeroRotator);
				}
				else
				{
					UMaterialInterface* Override = WallMaterials.IsValidIndex(Type) ? WallMaterials[Type].Get() : nullptr;
					GetISM(Type, JJWallColor(Type), Override, true)
						->AddInstance(FTransform(FRotator::ZeroRotator, CellCenter(X, Y, WallHeight * 0.5f), FVector(Cell / 100.f, Cell / 100.f, WallHeight / 100.f)));
				}
				continue;
			}

			GetISM(100, L.FloorColor, FloorMaterial, true)->AddInstance(FTransform(FRotator::ZeroRotator, CellCenter(X, Y, -10.f), TileScale));
			if (!Outdoor[I])
			{
				GetISM(101, L.CeilingColor, CeilingMaterial, true)->AddInstance(FTransform(FRotator::ZeroRotator, CellCenter(X, Y, WallHeight + 10.f), TileScale));
				if (X % 3 == 1 && Y % 3 == 1)
				{
					AddLamp(X, Y, L.LightColor);
				}
			}

			EJJEnemyKind EnemyKind;
			EJJPickup PickupType;
			if (C == 'P')
			{
				PlayerStart = CellCenter(X, Y, 110.f);
				PlayerYaw = L.Yaw;
			}
			else if (JJEnemyFromChar(C, EnemyKind))
			{
				const FTransform T(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), CellCenter(X, Y, 130.f));
				AJJEnemy* Enemy = World->SpawnActorDeferred<AJJEnemy>(AJJEnemy::StaticClass(), T, nullptr, nullptr,
					ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
				if (Enemy)
				{
					Enemy->Kind = EnemyKind;
					Enemy->FinishSpawning(T);
					TotalEnemies++;
				}
			}
			else if (JJPickupFromChar(C, PickupType))
			{
				AJJPickup::SpawnPickup(World, PickupType, CellCenter(X, Y, 40.f), 0, true);
				TotalItems++;
			}
			else if (C == 'o')
			{
				World->SpawnActor<AJJBarrel>(AJJBarrel::StaticClass(), CellCenter(X, Y), FRotator::ZeroRotator);
			}
		}
	}

	// Luz del cielo: atardecer rojizo en los niveles con zonas al aire libre.
	const bool bSky = L.Sky.Num() > 0;
	Sun->SetWorldRotation(FRotator(-14.f, 35.f, 0.f));
	Sun->SetLightColor(FLinearColor(1.f, 0.42f, 0.22f));
	Sun->SetIntensity(bSky ? 5.f : 0.f);
	Sun->SetAtmosphereSunLight(true);
	SkyLight->SetIntensity(bSky ? 1.f : 0.3f);
	Fog->SetFogDensity(0.035f);
	Fog->SetFogInscatteringColor(FLinearColor(0.18f, 0.05f, 0.03f));

	ComputeFlow();
}

void AJJLevelBuilder::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FlowTimer -= DeltaSeconds;
	if (FlowTimer <= 0.f)
	{
		FlowTimer = 0.4f;
		ComputeFlow();
	}
}

void AJJLevelBuilder::ComputeFlow()
{
	if (W == 0 || H == 0)
	{
		return;
	}
	for (int32& V : Flow)
	{
		V = -1;
	}
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}
	const FIntPoint S = WorldToCell(Player->GetActorLocation());
	if (S.X < 0 || S.Y < 0 || S.X >= W || S.Y >= H)
	{
		return;
	}
	// Búsqueda en anchura desde la casilla del jugador: cada casilla guarda su distancia.
	static const int32 DX[4] = { 1, -1, 0, 0 };
	static const int32 DY[4] = { 0, 0, 1, -1 };
	TArray<int32> Queue;
	Queue.Reserve(W * H);
	Flow[S.Y * W + S.X] = 0;
	Queue.Add(S.Y * W + S.X);
	for (int32 Head = 0; Head < Queue.Num(); ++Head)
	{
		const int32 C = Queue[Head];
		const int32 CX = C % W;
		const int32 CY = C / W;
		for (int32 K = 0; K < 4; ++K)
		{
			const int32 NX = CX + DX[K];
			const int32 NY = CY + DY[K];
			if (IsSolidCell(NX, NY))
			{
				continue;
			}
			const int32 N = NY * W + NX;
			if (Flow[N] >= 0)
			{
				continue;
			}
			Flow[N] = Flow[C] + 1;
			Queue.Add(N);
		}
	}
}

bool AJJLevelBuilder::NextStepTowardsPlayer(const FVector& From, FVector& OutTarget) const
{
	const FIntPoint C = WorldToCell(From);
	if (C.X < 0 || C.Y < 0 || C.X >= W || C.Y >= H || Flow.Num() != W * H)
	{
		return false;
	}
	static const int32 DX[4] = { 1, -1, 0, 0 };
	static const int32 DY[4] = { 0, 0, 1, -1 };
	int32 Best = -1;
	int32 BestValue = Flow[C.Y * W + C.X] >= 0 ? Flow[C.Y * W + C.X] : MAX_int32;
	for (int32 K = 0; K < 4; ++K)
	{
		const int32 NX = C.X + DX[K];
		const int32 NY = C.Y + DY[K];
		if (NX < 0 || NY < 0 || NX >= W || NY >= H)
		{
			continue;
		}
		const int32 V = Flow[NY * W + NX];
		if (V >= 0 && V < BestValue)
		{
			BestValue = V;
			Best = NY * W + NX;
		}
	}
	if (Best < 0)
	{
		return false;
	}
	OutTarget = CellCenter(Best % W, Best / W, From.Z);
	return true;
}
