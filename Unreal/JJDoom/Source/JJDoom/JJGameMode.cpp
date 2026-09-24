#include "JJGameMode.h"
#include "JJLevels.h"
#include "JJLevelBuilder.h"
#include "JJCharacter.h"
#include "JJEnemy.h"
#include "JJHUD.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AJJGameMode::AJJGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AJJCharacter::StaticClass();
	HUDClass = AJJHUD::StaticClass();
	BuilderClass = AJJLevelBuilder::StaticClass();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	BasicMeshes = { CubeFinder.Object, SphereFinder.Object, CylinderFinder.Object, ConeFinder.Object };
	BasicMaterial = MaterialFinder.Object;
}

AJJGameMode* AJJGameMode::Get(const UObject* WorldContext)
{
	return Cast<AJJGameMode>(UGameplayStatics::GetGameMode(WorldContext));
}

void AJJGameMode::StartPlay()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UClass* Class = BuilderClass ? BuilderClass.Get() : AJJLevelBuilder::StaticClass();
	Builder = GetWorld()->SpawnActor<AJJLevelBuilder>(Class, FTransform::Identity, Params);
	StartLevel(0, true);
	Super::StartPlay();
}

void AJJGameMode::StartLevel(int32 Index, bool bNewGame)
{
	if (!Builder)
	{
		return;
	}
	LevelIndex = Index;
	Builder->Build(Index);
	Kills = 0;
	TotalKills = Builder->TotalEnemies;
	Items = 0;
	TotalItems = Builder->TotalItems;
	LevelTime = 0.f;
	ExitTime = 0.f;
	StateTime = 0.f;
	State = EJJGameState::Playing;

	const FJJLevelData& L = JJLevels::Get()[Index];
	ShowMessage(L.bBoss ? L.Name + TEXT(" - ¡Mata al BARÓN!") : L.Name, 4.f);

	if (AJJCharacter* Player = Cast<AJJCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		if (bNewGame)
		{
			Player->Inv = FJJInventory();
		}
		Player->Revive();
		Snapshot = Player->Inv;
		PlacePlayer(Player);
	}
	else if (bNewGame)
	{
		Snapshot = FJJInventory();
	}
}

void AJJGameMode::PlacePlayer(AJJCharacter* Player)
{
	if (!Player || !Builder)
	{
		return;
	}
	const FRotator Facing(0.f, Builder->GetPlayerYaw(), 0.f);
	Player->SetActorLocationAndRotation(Builder->GetPlayerStart(), Facing, false, nullptr, ETeleportType::TeleportPhysics);
	Player->GetCharacterMovement()->StopMovementImmediately();
	if (AController* Controller = Player->GetController())
	{
		Controller->SetControlRotation(Facing);
	}
}

void AJJGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	MessageTime -= DeltaSeconds;
	StateTime += DeltaSeconds;
	if (State == EJJGameState::Playing)
	{
		LevelTime += DeltaSeconds;
	}
	if (State == EJJGameState::Exiting)
	{
		ExitTime -= DeltaSeconds;
		if (ExitTime <= 0.f)
		{
			State = EJJGameState::Intermission;
			StateTime = 0.f;
		}
	}
}

void AJJGameMode::ShowMessage(const FString& Text, float Seconds)
{
	Message = Text;
	MessageTime = Seconds;
}

void AJJGameMode::AddKill(AJJEnemy* Enemy)
{
	Kills++;
	if (Enemy && Enemy->IsBoss())
	{
		ShowMessage(TEXT("¡El BARÓN ha caído! La SALIDA está abierta"), 5.f);
	}
}

void AJJGameMode::AlertEnemies(const FVector& Where, float Radius)
{
	for (TActorIterator<AJJEnemy> It(GetWorld()); It; ++It)
	{
		if (FVector::Dist(It->GetActorLocation(), Where) < Radius)
		{
			It->Wake();
		}
	}
}

bool AJJGameMode::TryExit()
{
	if (State != EJJGameState::Playing)
	{
		return false;
	}
	if (JJLevels::Get()[LevelIndex].bBoss)
	{
		for (TActorIterator<AJJEnemy> It(GetWorld()); It; ++It)
		{
			if (It->IsBoss() && It->IsAlive())
			{
				ShowMessage(TEXT("La SALIDA está sellada. ¡Mata al BARÓN!"));
				return false;
			}
		}
	}
	State = EJJGameState::Exiting;
	ExitTime = 1.f;
	ShowMessage(TEXT("¡Salida activada!"));
	return true;
}

void AJJGameMode::OnPlayerDied()
{
	State = EJJGameState::Dead;
	StateTime = 0.f;
	ShowMessage(TEXT("HAS MUERTO"));
}

void AJJGameMode::OnContinuePressed()
{
	AJJCharacter* Player = Cast<AJJCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (State == EJJGameState::Intermission && StateTime > 0.5f)
	{
		if (LevelIndex + 1 < JJLevels::Get().Num())
		{
			StartLevel(LevelIndex + 1, false);
		}
		else
		{
			State = EJJGameState::Victory;
			StateTime = 0.f;
		}
	}
	else if (State == EJJGameState::Victory && StateTime > 1.f)
	{
		StartLevel(0, true);
	}
	else if (State == EJJGameState::Dead && StateTime > 1.f)
	{
		// Se reintenta el nivel con el equipo que se tenía al empezarlo.
		if (Player)
		{
			Player->Inv = Snapshot;
		}
		StartLevel(LevelIndex, false);
	}
}
