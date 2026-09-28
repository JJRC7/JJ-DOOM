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
#include "Engine/PostProcessVolume.h"
#include "Misc/PackageName.h"
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

	// Materiales del paquete gratuito "Starter Content" (si se ha añadido al proyecto).
	UMaterialInterface* JJStarterMaterial(const TCHAR* Name)
	{
		const FString Package = FString::Printf(TEXT("/Game/StarterContent/Materials/%s"), Name);
		if (!FPackageName::DoesPackageExist(Package))
		{
			return nullptr;
		}
		return LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("%s.%s"), *Package, Name));
	}

	const TCHAR* JJStarterWallName(int32 Type)
	{
		switch (Type)
		{
		case 1: return TEXT("M_Brick_Clay_Old");
		case 2: return TEXT("M_Metal_Steel");
		case 3: return TEXT("M_Brick_Cut_Stone");
		case 4: return TEXT("M_Brick_Hewn_Stone");
		case 10: return TEXT("M_Tech_Panel");
		case 11: return TEXT("M_Wood_Pine");
		case 12: return TEXT("M_Rock_Marble_Polished");
		default: return nullptr;
		}
	}

	const TCHAR* JJStarterFloorName(int32 Level)
	{
		static const TCHAR* Names[] = { TEXT("M_Concrete_Tiles"), TEXT("M_Cobblestone_Rough"), TEXT("M_Concrete_Poured"),
			TEXT("M_Ceramic_Tile_Checker"), TEXT("M_Rock_Basalt") };
		return Names[FMath::Clamp(Level, 0, 4)];
	}

	const TCHAR* JJStarterCeilingName(int32 Level)
	{
		static const TCHAR* Names[] = { TEXT("M_Concrete_Panels"), TEXT("M_Rock_Basalt"), TEXT("M_Concrete_Panels"),
			TEXT("M_Wood_Walnut"), TEXT("M_Rock_Basalt") };
		return Names[FMath::Clamp(Level, 0, 4)];
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
	Fog->bEnableVolumetricFog = true;

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

void AJJLevelBuilder::AddBox(int32 Key, const FLinearColor& Color, UMaterialInterface* Material, const FVector& Center, const FVector& Size, bool bCollision)
{
	GetISM(Key, Color, Material, bCollision)->AddInstance(FTransform(FRotator::ZeroRotator, Center, Size / 100.f));
}

UMaterialInterface* AJJLevelBuilder::WallMaterialFor(int32 Type) const
{
	if (WallMaterials.IsValidIndex(Type) && WallMaterials[Type])
	{
		return WallMaterials[Type].Get();
	}
	const TCHAR* Name = JJStarterWallName(Type);
	return Name ? JJStarterMaterial(Name) : nullptr;
}

void AJJLevelBuilder::SetupPostProcess()
{
	if (PostVolume)
	{
		return;
	}
	FActorSpawnParameters Params;
	Params.Owner = this;
	PostVolume = GetWorld()->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform::Identity, Params);
	if (!PostVolume)
	{
		return;
	}
	// Aspecto de película: resplandor, viñeta, grano, aberración cromática y algo más de contraste.
	PostVolume->bUnbound = true;
	FPostProcessSettings& S = PostVolume->Settings;
	S.bOverride_BloomIntensity = true;
	S.BloomIntensity = 1.2f;
	S.bOverride_VignetteIntensity = true;
	S.VignetteIntensity = 0.6f;
	S.bOverride_FilmGrainIntensity = true;
	S.FilmGrainIntensity = 0.2f;
	S.bOverride_SceneFringeIntensity = true;
	S.SceneFringeIntensity = 1.2f;
	S.bOverride_ColorContrast = true;
	S.ColorContrast = FVector4(1.12f, 1.12f, 1.12f, 1.f);
	S.bOverride_ColorSaturation = true;
	S.ColorSaturation = FVector4(1.08f, 1.f, 0.92f, 1.f);
	S.bOverride_AmbientOcclusionIntensity = true;
	S.AmbientOcclusionIntensity = 0.8f;
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
	Light->SetVolumetricScatteringIntensity(1.5f);
	Light->RegisterComponent();
	Lamps.Add(Light);
	// Algunas lámparas parpadean.
	if (FMath::FRand() < 0.2f)
	{
		FlickerLamps.Add(Light);
		FlickerBase.Add(LampCandelas);
	}

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
	FlickerLamps.Empty();
	FlickerBase.Empty();
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
	const int32 LevelNum = FMath::Clamp(LevelIndex, 0, Levels.Num() - 1);
	UMaterialInterface* FloorMat = FloorMaterial ? FloorMaterial.Get() : JJStarterMaterial(JJStarterFloorName(LevelNum));
	UMaterialInterface* CeilingMat = CeilingMaterial ? CeilingMaterial.Get() : JJStarterMaterial(JJStarterCeilingName(LevelNum));
	UMaterialInterface* TrimMat = JJStarterMaterial(TEXT("M_Metal_Rust"));
	UMaterialInterface* BeamMat = JJStarterMaterial(TEXT("M_Wood_Oak"));
	UMaterialInterface* DoorMat = JJStarterMaterial(TEXT("M_Metal_Burnished_Steel"));
	const float Half = Cell * 0.5f;

	// Suelo y techo en baldosas de 2x2 por casilla (las texturas se ven a mejor escala).
	auto AddFloorAndCeiling = [&](int32 X, int32 Y, bool bCeiling)
	{
		for (int32 IX = 0; IX < 2; ++IX)
		{
			for (int32 IY = 0; IY < 2; ++IY)
			{
				const FVector Offset((IX - 0.5f) * Half, (IY - 0.5f) * Half, 0.f);
				AddBox(100, L.FloorColor, FloorMat, CellCenter(X, Y, -10.f) + Offset, FVector(Half, Half, 20.f), true);
				if (bCeiling)
				{
					AddBox(101, L.CeilingColor, CeilingMat, CellCenter(X, Y, WallHeight + 10.f) + Offset, FVector(Half, Half, 20.f), true);
				}
			}
		}
	};
	auto IsOpen = [&](int32 X, int32 Y)
	{
		const int32 T = JJWallType(CharAt(X, Y));
		return T == 0 || JJIsDoor(T);
	};

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
						Door->Setup(Type == 7 ? EJJKey::Red : Type == 9 ? EJJKey::Blue : EJJKey::None, bAlongX, DoorMat);
						Doors.Add(I, Door);
					}
					// Suelo y techo bajo la puerta (se esconde en el techo al abrirse).
					AddFloorAndCeiling(X, Y, true);
				}
				else if (Type == 5)
				{
					World->SpawnActor<AJJExitSwitch>(AJJExitSwitch::StaticClass(), CellCenter(X, Y), FRotator::ZeroRotator);
				}
				else
				{
					// Solo las paredes que se ven (junto a una casilla abierta), en bloques de 2x2x2
					// para que ladrillos y paneles tengan un tamaño realista.
					bool bVisible = false;
					for (int32 DY = -1; DY <= 1 && !bVisible; ++DY)
					{
						for (int32 DX = -1; DX <= 1 && !bVisible; ++DX)
						{
							bVisible = (DX != 0 || DY != 0) && IsOpen(X + DX, Y + DY);
						}
					}
					if (bVisible)
					{
						UMaterialInterface* Material = WallMaterialFor(Type);
						for (int32 IX = 0; IX < 2; ++IX)
						{
							for (int32 IY = 0; IY < 2; ++IY)
							{
								for (int32 IZ = 0; IZ < 2; ++IZ)
								{
									const FVector Center = CellCenter(X, Y, (IZ + 0.5f) * WallHeight * 0.5f) + FVector((IX - 0.5f) * Half, (IY - 0.5f) * Half, 0.f);
									AddBox(Type, JJWallColor(Type), Material, Center, FVector(Half, Half, WallHeight * 0.5f), true);
								}
							}
						}
					}
				}
				continue;
			}

			AddFloorAndCeiling(X, Y, !Outdoor[I]);
			if (!Outdoor[I])
			{
				if (X % 3 == 1 && Y % 3 == 1)
				{
					AddLamp(X, Y, L.LightColor);
				}
				// Vigas del techo cada dos casillas.
				if (X % 2 == 0)
				{
					AddBox(104, FLinearColor(0.12f, 0.07f, 0.04f), BeamMat, CellCenter(X, Y, WallHeight - 18.f), FVector(28.f, Cell, 36.f), false);
				}
			}
			// Zócalo y moldura en las paredes que rodean la casilla.
			static const int32 NX[4] = { 1, -1, 0, 0 };
			static const int32 NY[4] = { 0, 0, 1, -1 };
			for (int32 K = 0; K < 4; ++K)
			{
				const int32 NT = JJWallType(CharAt(X + NX[K], Y + NY[K]));
				if (NT == 0 || JJIsDoor(NT) || NT == 5)
				{
					continue;
				}
				const FVector Edge = CellCenter(X, Y) + FVector(NX[K] * (Half - 6.f), NY[K] * (Half - 6.f), 0.f);
				const FVector Size = NX[K] != 0 ? FVector(12.f, Cell, 1.f) : FVector(Cell, 12.f, 1.f);
				AddBox(103, FLinearColor(0.08f, 0.07f, 0.06f), TrimMat, Edge + FVector(0.f, 0.f, 16.f), FVector(Size.X, Size.Y, 32.f), false);
				if (!Outdoor[I])
				{
					AddBox(103, FLinearColor(0.08f, 0.07f, 0.06f), TrimMat, Edge + FVector(0.f, 0.f, WallHeight - 14.f), FVector(Size.X, Size.Y, 28.f), false);
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

	SetupPostProcess();

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
	FlickerTimer -= DeltaSeconds;
	if (FlickerTimer <= 0.f)
	{
		FlickerTimer = 0.06f;
		for (int32 I = 0; I < FlickerLamps.Num(); ++I)
		{
			if (FlickerLamps[I])
			{
				const float Factor = FMath::FRand() < 0.12f ? FMath::FRandRange(0.02f, 0.4f) : 1.f;
				FlickerLamps[I]->SetIntensity(FlickerBase[I] * Factor);
			}
		}
	}
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
