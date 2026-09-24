#include "JJExitSwitch.h"
#include "JJAssets.h"
#include "JJGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AJJExitSwitch::AJJExitSwitch()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Block = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Block"));
	Block->SetupAttachment(RootComponent);
	Panel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel"));
	Panel->SetupAttachment(RootComponent);
}

void AJJExitSwitch::BeginPlay()
{
	Super::BeginPlay();
	JJAssets::SetupPart(Block, JJAssets::Cube(), FLinearColor(0.18f, 0.19f, 0.21f), FVector(4.f), FVector(0.f, 0.f, 200.f));
	Block->SetCollisionProfileName(TEXT("BlockAll"));
	JJAssets::SetupPart(Panel, JJAssets::Cube(), FLinearColor(0.9f, 0.02f, 0.02f), FVector(4.06f, 4.06f, 0.8f), FVector(0.f, 0.f, 190.f));
}

void AJJExitSwitch::Activate()
{
	if (bUsed)
	{
		return;
	}
	AJJGameMode* GM = AJJGameMode::Get(this);
	if (GM && GM->TryExit())
	{
		bUsed = true;
		Panel->SetMaterial(0, JJAssets::ColorMaterial(Panel, FLinearColor(0.05f, 0.9f, 0.05f)));
	}
}
