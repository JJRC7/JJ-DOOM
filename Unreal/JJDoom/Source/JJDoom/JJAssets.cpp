#include "JJAssets.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	UStaticMesh* JJLoadMesh(const TCHAR* Path)
	{
		return LoadObject<UStaticMesh>(nullptr, Path);
	}
}

UStaticMesh* JJAssets::Cube()     { return JJLoadMesh(TEXT("/Engine/BasicShapes/Cube.Cube")); }
UStaticMesh* JJAssets::Sphere()   { return JJLoadMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")); }
UStaticMesh* JJAssets::Cylinder() { return JJLoadMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")); }
UStaticMesh* JJAssets::Cone()     { return JJLoadMesh(TEXT("/Engine/BasicShapes/Cone.Cone")); }

UMaterialInstanceDynamic* JJAssets::ColorMaterial(UObject* Outer, const FLinearColor& Color)
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Outer);
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
	}
	return Material;
}

void JJAssets::SetupPart(UStaticMeshComponent* Part, UStaticMesh* Mesh, const FLinearColor& Color, const FVector& Scale,
	const FVector& Location, const FRotator& Rotation)
{
	if (!Part)
	{
		return;
	}
	Part->SetStaticMesh(Mesh);
	Part->SetMaterial(0, ColorMaterial(Part, Color));
	Part->SetRelativeScale3D(Scale);
	Part->SetRelativeLocationAndRotation(Location, Rotation);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetVisibility(true);
}
