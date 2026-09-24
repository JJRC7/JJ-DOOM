#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JJProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UProjectileMovementComponent;

// Bolas de fuego, plasma y cohetes.
UCLASS()
class JJDOOM_API AJJProjectile : public AActor
{
	GENERATED_BODY()

public:
	AJJProjectile();

	static AJJProjectile* Fire(UWorld* World, AActor* Shooter, const FVector& From, const FVector& Direction, float Damage,
		float Speed, const FLinearColor& Color, bool bExplosive, float Size);

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Ball;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> Light;

	UPROPERTY()
	TObjectPtr<UProjectileMovementComponent> Movement;

	float Damage = 10.f;
	bool bExplosive = false;
	bool bDone = false;
	FLinearColor Color = FLinearColor::White;
};
