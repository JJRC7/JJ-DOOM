#include "JJGameMode.h"
#include "JJLevels.h"
#include "JJLevelBuilder.h"
#include "JJCharacter.h"
#include "JJEnemy.h"
#include "JJHUD.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Misc/PackageName.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	// Dificultades: vida y daño de los enemigos.
	const float JJHealthMul[] = { 1.2f, 2.5f, 3.5f, 4.5f };
	const float JJDamageMul[] = { 0.7f, 1.3f, 1.7f, 2.2f };
	const TCHAR* JJOptionsSection = TEXT("JJDoom");

	UGameUserSettings* JJUserSettings()
	{
		return GEngine ? GEngine->GetGameUserSettings() : nullptr;
	}
}

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

const FJJMonsterVisual* AJJGameMode::FindMonsterVisual(EJJEnemyKind Kind) const
{
	const FJJMonsterVisual* Visual = MonsterVisuals.Find(Kind);
	return (Visual && Visual->Mesh != nullptr) ? Visual : nullptr;
}

const FJJWeaponVisual* AJJGameMode::FindWeaponVisual(EJJWeapon Weapon) const
{
	const FJJWeaponVisual* Visual = WeaponVisuals.Find(Weapon);
	return (Visual && (Visual->StaticMesh != nullptr || Visual->SkeletalMesh != nullptr)) ? Visual : nullptr;
}

AJJGameMode* AJJGameMode::Get(const UObject* WorldContext)
{
	return Cast<AJJGameMode>(UGameplayStatics::GetGameMode(WorldContext));
}

void AJJGameMode::StartPlay()
{
	// Si el juego usa este modo de juego base pero existe /Game/BP_GameMode (creado en el editor),
	// se copian de él los modelos de monstruos y armas y el constructor de niveles.
	// Así funciona aunque "Default GameMode" en la configuración del proyecto no apunte a BP_GameMode.
	if (GetClass() == AJJGameMode::StaticClass() && FPackageName::DoesPackageExist(FString(TEXT("/Game/BP_GameMode"))))
	{
		if (UClass* BlueprintClass = LoadClass<AJJGameMode>(nullptr, TEXT("/Game/BP_GameMode.BP_GameMode_C")))
		{
			const AJJGameMode* Defaults = BlueprintClass->GetDefaultObject<AJJGameMode>();
			if (MonsterVisuals.Num() == 0)
			{
				MonsterVisuals = Defaults->MonsterVisuals;
			}
			if (WeaponVisuals.Num() == 0)
			{
				WeaponVisuals = Defaults->WeaponVisuals;
			}
			if (Defaults->BuilderClass)
			{
				BuilderClass = Defaults->BuilderClass;
			}
		}
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UClass* Class = BuilderClass ? BuilderClass.Get() : AJJLevelBuilder::StaticClass();
	Builder = GetWorld()->SpawnActor<AJJLevelBuilder>(Class, FTransform::Identity, Params);
	LoadOptions();
	StartLevel(0, true);
	bFreshLevel = true;
	Super::StartPlay();
	// El juego empieza en el menú principal, con el primer nivel de fondo.
	OpenMenu(EJJMenu::Main);
}

// ---------------------------------------------------------------------------------------------
// Ajustes
// ---------------------------------------------------------------------------------------------

const TCHAR* AJJGameMode::DifficultyName(int32 Level)
{
	static const TCHAR* Names[] = { TEXT("FÁCIL"), TEXT("NORMAL"), TEXT("DIFÍCIL"), TEXT("PESADILLA") };
	return Names[FMath::Clamp(Level, 0, 3)];
}

void AJJGameMode::LoadOptions()
{
	if (GConfig)
	{
		GConfig->GetInt(JJOptionsSection, TEXT("Difficulty"), Difficulty, GGameUserSettingsIni);
		GConfig->GetInt(JJOptionsSection, TEXT("ScreenPercentage"), ScreenPercentage, GGameUserSettingsIni);
		GConfig->GetInt(JJOptionsSection, TEXT("Brightness"), SavedBrightness, GGameUserSettingsIni);
		GConfig->GetBool(JJOptionsSection, TEXT("ShowFPS"), bShowFPS, GGameUserSettingsIni);
	}
	SetDifficulty(Difficulty);
	SetScreenPercentage(ScreenPercentage);
	if (SavedBrightness >= 0)
	{
		SetBrightness(SavedBrightness);
	}
}

void AJJGameMode::SaveOptions() const
{
	if (!GConfig)
	{
		return;
	}
	GConfig->SetInt(JJOptionsSection, TEXT("Difficulty"), Difficulty, GGameUserSettingsIni);
	GConfig->SetInt(JJOptionsSection, TEXT("ScreenPercentage"), ScreenPercentage, GGameUserSettingsIni);
	GConfig->SetInt(JJOptionsSection, TEXT("Brightness"), GetBrightness(), GGameUserSettingsIni);
	GConfig->SetBool(JJOptionsSection, TEXT("ShowFPS"), bShowFPS, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void AJJGameMode::SetDifficulty(int32 Level)
{
	Difficulty = FMath::Clamp(Level, 0, 3);
	EnemyHealthMultiplier = JJHealthMul[Difficulty];
	EnemyDamageMultiplier = JJDamageMul[Difficulty];
	// Los enemigos que ya están en el nivel también cambian.
	for (TActorIterator<AJJEnemy> It(GetWorld()); It; ++It)
	{
		It->ApplyDifficulty(EnemyHealthMultiplier, EnemyDamageMultiplier);
	}
	SaveOptions();
}

int32 AJJGameMode::GetBrightness() const
{
	return Builder ? Builder->ChangeBrightness(0.f) : 8;
}

void AJJGameMode::SetBrightness(int32 Level)
{
	if (Builder)
	{
		Builder->ChangeBrightness(static_cast<float>(FMath::Clamp(Level, 0, 16) - GetBrightness()));
		SaveOptions();
	}
}

void AJJGameMode::SetScreenPercentage(int32 Percent)
{
	ScreenPercentage = FMath::Clamp(Percent, 50, 100);
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
	{
		CVar->Set(static_cast<float>(ScreenPercentage), ECVF_SetByConsole);
	}
	SaveOptions();
}

// ---------------------------------------------------------------------------------------------
// Menús
// ---------------------------------------------------------------------------------------------

void AJJGameMode::GetMenuItems(TArray<EJJMenuItem>& Out) const
{
	Out.Reset();
	switch (Menu)
	{
	case EJJMenu::Main:
		Out = { EJJMenuItem::Play, EJJMenuItem::Difficulty, EJJMenuItem::Options, EJJMenuItem::Quit };
		break;
	case EJJMenu::Pause:
		Out = { EJJMenuItem::Resume, EJJMenuItem::Restart, EJJMenuItem::Difficulty, EJJMenuItem::Options, EJJMenuItem::MainMenu, EJJMenuItem::Quit };
		break;
	case EJJMenu::Options:
		Out = { EJJMenuItem::Quality, EJJMenuItem::Resolution, EJJMenuItem::Brightness, EJJMenuItem::VSync, EJJMenuItem::ShowFPS, EJJMenuItem::Back };
		break;
	default:
		break;
	}
}

FString AJJGameMode::GetMenuLabel(EJJMenuItem Item) const
{
	switch (Item)
	{
	case EJJMenuItem::Play: return TEXT("JUGAR");
	case EJJMenuItem::Resume: return TEXT("CONTINUAR");
	case EJJMenuItem::Restart: return TEXT("REINICIAR NIVEL");
	case EJJMenuItem::Difficulty: return TEXT("DIFICULTAD");
	case EJJMenuItem::Options: return TEXT("OPCIONES GRÁFICAS");
	case EJJMenuItem::Quality: return TEXT("CALIDAD");
	case EJJMenuItem::Resolution: return TEXT("RESOLUCIÓN");
	case EJJMenuItem::Brightness: return TEXT("BRILLO");
	case EJJMenuItem::VSync: return TEXT("SINCRONÍA VERTICAL");
	case EJJMenuItem::ShowFPS: return TEXT("MOSTRAR FPS");
	case EJJMenuItem::Back: return TEXT("VOLVER");
	case EJJMenuItem::MainMenu: return TEXT("MENÚ PRINCIPAL");
	case EJJMenuItem::Quit: return TEXT("SALIR DEL JUEGO");
	}
	return FString();
}

FString AJJGameMode::GetMenuValue(EJJMenuItem Item) const
{
	const UGameUserSettings* US = JJUserSettings();
	switch (Item)
	{
	case EJJMenuItem::Difficulty:
		return DifficultyName(Difficulty);
	case EJJMenuItem::Quality:
	{
		static const TCHAR* Names[] = { TEXT("BAJA"), TEXT("MEDIA"), TEXT("ALTA"), TEXT("ÉPICA"), TEXT("CINE") };
		const int32 Level = US ? US->GetOverallScalabilityLevel() : -1;
		return Level >= 0 && Level <= 4 ? FString(Names[Level]) : FString(TEXT("PERSONALIZADA"));
	}
	case EJJMenuItem::Resolution:
		return FString::Printf(TEXT("%d%%"), ScreenPercentage);
	case EJJMenuItem::Brightness:
		return FString::Printf(TEXT("%d / 16"), GetBrightness());
	case EJJMenuItem::VSync:
		return US && US->IsVSyncEnabled() ? TEXT("SÍ") : TEXT("NO");
	case EJJMenuItem::ShowFPS:
		return bShowFPS ? TEXT("SÍ") : TEXT("NO");
	default:
		return FString();
	}
}

FString AJJGameMode::GetMenuHint(EJJMenuItem Item) const
{
	switch (Item)
	{
	case EJJMenuItem::Play: return TEXT("Empieza una partida nueva desde el primer nivel");
	case EJJMenuItem::Resume: return TEXT("Vuelve a la partida");
	case EJJMenuItem::Restart: return TEXT("Empiezas el nivel otra vez con el equipo que tenías al entrar");
	case EJJMenuItem::Difficulty:
		return FString::Printf(TEXT("Vida de los enemigos x%.1f   ·   Daño de los enemigos x%.1f"), EnemyHealthMultiplier, EnemyDamageMultiplier);
	case EJJMenuItem::Options: return TEXT("Calidad, resolución, brillo, sincronía vertical y FPS");
	case EJJMenuItem::Quality: return TEXT("BAJA = más fluido   ·   ÉPICA = más bonito (usa más tarjeta gráfica)");
	case EJJMenuItem::Resolution: return TEXT("Menos porcentaje = más FPS, pero la imagen se ve algo más borrosa");
	case EJJMenuItem::Brightness: return TEXT("También se cambia en el juego con las teclas + y -");
	case EJJMenuItem::VSync: return TEXT("Quita los cortes de imagen, pero puede añadir algo de retraso");
	case EJJMenuItem::ShowFPS: return TEXT("Muestra las imágenes por segundo arriba a la derecha");
	case EJJMenuItem::Back: return TEXT("Vuelve al menú anterior");
	case EJJMenuItem::MainMenu: return TEXT("Abandona la partida y vuelve al menú de inicio");
	case EJJMenuItem::Quit: return TEXT("Cierra el juego");
	}
	return FString();
}

void AJJGameMode::OpenMenu(EJJMenu NewMenu)
{
	Menu = NewMenu;
	MenuIndex = 0;
	if (AJJCharacter* Player = Cast<AJJCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->CancelFire();
	}
	SyncMenuState();
}

void AJJGameMode::CloseMenu()
{
	Menu = EJJMenu::None;
	SyncMenuState();
}

void AJJGameMode::SyncMenuState()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}
	const bool bOpen = Menu != EJJMenu::None;
	if (UGameplayStatics::IsGamePaused(this) != bOpen)
	{
		PC->SetPause(bOpen);
	}
	if (PC->bShowMouseCursor != bOpen)
	{
		PC->bShowMouseCursor = bOpen;
		if (bOpen)
		{
			FInputModeGameAndUI Mode;
			Mode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(Mode);
		}
		else
		{
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
}

void AJJGameMode::TogglePause()
{
	switch (Menu)
	{
	case EJJMenu::None:
		OpenMenu(EJJMenu::Pause);
		break;
	case EJJMenu::Pause:
		CloseMenu();
		break;
	case EJJMenu::Options:
		MenuBack();
		break;
	default:
		break;
	}
}

void AJJGameMode::MenuBack()
{
	if (Menu == EJJMenu::Options)
	{
		OpenMenu(MenuParent);
		TArray<EJJMenuItem> Items;
		GetMenuItems(Items);
		MenuIndex = FMath::Max(0, Items.IndexOfByKey(EJJMenuItem::Options));
	}
	else if (Menu == EJJMenu::Pause)
	{
		CloseMenu();
	}
}

void AJJGameMode::MenuMove(int32 Dir)
{
	TArray<EJJMenuItem> Items;
	GetMenuItems(Items);
	if (Items.Num() > 0)
	{
		MenuIndex = (MenuIndex + Dir + Items.Num()) % Items.Num();
	}
}

void AJJGameMode::MenuAdjust(int32 Dir)
{
	TArray<EJJMenuItem> Items;
	GetMenuItems(Items);
	if (!Items.IsValidIndex(MenuIndex))
	{
		return;
	}
	UGameUserSettings* US = JJUserSettings();
	switch (Items[MenuIndex])
	{
	case EJJMenuItem::Difficulty:
		SetDifficulty((Difficulty + Dir + 4) % 4);
		break;
	case EJJMenuItem::Quality:
		if (US)
		{
			const int32 Current = US->GetOverallScalabilityLevel();
			US->SetOverallScalabilityLevel(((Current < 0 ? 1 : Current) + Dir + 4) % 4);
			US->ApplyNonResolutionSettings();
			US->SaveSettings();
			SetScreenPercentage(ScreenPercentage); // la calidad no debe pisar la resolución elegida
		}
		break;
	case EJJMenuItem::Resolution:
	{
		int32 Next = ScreenPercentage + Dir * 5;
		if (Next > 100) Next = 50;
		if (Next < 50) Next = 100;
		SetScreenPercentage(Next);
		break;
	}
	case EJJMenuItem::Brightness:
		SetBrightness(FMath::Clamp(GetBrightness() + Dir, 0, 16));
		break;
	case EJJMenuItem::VSync:
		if (US)
		{
			US->SetVSyncEnabled(!US->IsVSyncEnabled());
			US->ApplyNonResolutionSettings();
			US->SaveSettings();
			SetScreenPercentage(ScreenPercentage);
		}
		break;
	case EJJMenuItem::ShowFPS:
		bShowFPS = !bShowFPS;
		SaveOptions();
		break;
	default:
		break;
	}
}

void AJJGameMode::MenuAccept()
{
	TArray<EJJMenuItem> Items;
	GetMenuItems(Items);
	if (!Items.IsValidIndex(MenuIndex))
	{
		return;
	}
	switch (Items[MenuIndex])
	{
	case EJJMenuItem::Play:
		CloseMenu();
		if (!bFreshLevel)
		{
			StartLevel(0, true);
		}
		bFreshLevel = false;
		break;
	case EJJMenuItem::Resume:
		CloseMenu();
		break;
	case EJJMenuItem::Restart:
		CloseMenu();
		RestartLevel();
		break;
	case EJJMenuItem::Options:
		MenuParent = Menu;
		OpenMenu(EJJMenu::Options);
		break;
	case EJJMenuItem::Back:
		MenuBack();
		break;
	case EJJMenuItem::MainMenu:
		StartLevel(0, true);
		bFreshLevel = true;
		OpenMenu(EJJMenu::Main);
		break;
	case EJJMenuItem::Quit:
		UKismetSystemLibrary::QuitGame(this, GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
		break;
	default:
		// Opciones con valor: Enter o clic pasan al siguiente valor.
		MenuAdjust(1);
		break;
	}
}

void AJJGameMode::RestartLevel()
{
	if (AJJCharacter* Player = Cast<AJJCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->Inv = Snapshot;
	}
	StartLevel(LevelIndex, false);
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
		RestartLevel();
	}
}
