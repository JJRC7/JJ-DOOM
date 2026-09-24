#include "JJDoor.h"
#include "JJAssets.h"
#include "JJCharacter.h"
#include "JJGameMode.h"
#include "Components/StaticMeshComponent.h"

AJJDoor::AJJDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Panel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel"));
	Panel->SetupAttachment(RootComponent);
	Band = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Band"));
	Band->SetupAttachment(RootComponent);
}

void AJJDoor::Setup(EJJKey InKey, bool bAlongX)
{
	Key = InKey;
	const FVector PanelScale = bAlongX ? FVector(4.f, 0.5f, 4.f) : FVector(0.5f, 4.f, 4.f);
	JJAssets::SetupPart(Panel, JJAssets::Cube(), FLinearColor(0.22f, 0.23f, 0.25f), PanelScale, FVector(0.f, 0.f, 200.f));
	Panel->SetCollisionProfileName(TEXT("BlockAll"));

	const FLinearColor BandColor = Key == EJJKey::Red ? FLinearColor(0.8f, 0.02f, 0.02f)
		: Key == EJJKey::Blue ? FLinearColor(0.02f, 0.1f, 0.9f) : FLinearColor(0.85f, 0.6f, 0.02f);
	const FVector BandScale = bAlongX ? FVector(3.6f, 0.56f, 0.35f) : FVector(0.56f, 3.6f, 0.35f);
	JJAssets::SetupPart(Band, JJAssets::Cube(), BandColor, BandScale, FVector(0.f, 0.f, 200.f));
}

bool AJJDoor::TryOpen(AJJCharacter* By)
{
	if (bOpening)
	{
		return true;
	}
	AJJGameMode* GM = AJJGameMode::Get(this);
	if (Key == EJJKey::Red && !(By && By->bRedKey))
	{
		if (GM) GM->ShowMessage(TEXT("Necesitas la LLAVE ROJA para abrir esta puerta"));
		return false;
	}
	if (Key == EJJKey::Blue && !(By && By->bBlueKey))
	{
		if (GM) GM->ShowMessage(TEXT("Necesitas la LLAVE AZUL para abrir esta puerta"));
		return false;
	}
	bOpening = true;
	return true;
}

void AJJDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bOpening || OpenAlpha >= 1.f)
	{
		return;
	}
	OpenAlpha = FMath::Min(1.f, OpenAlpha + DeltaSeconds * 4.f);
	const FVector Location(0.f, 0.f, 200.f + OpenAlpha * 390.f);
	Panel->SetRelativeLocation(Location);
	Band->SetRelativeLocation(Location);
}
