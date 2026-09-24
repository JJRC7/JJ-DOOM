#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JJTypes.h"
#include "JJEnemy.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

// Monstruo con IA sencilla: espera, persigue (en línea recta si te ve, siguiendo el mapa si no),
// ataca, se estremece al recibir daño y muere.
UCLASS()
class JJDOOM_API AJJEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AJJEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JJ")
	EJJEnemyKind Kind = EJJEnemyKind::Zombie;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void Wake();
	bool IsAlive() const { return State != EJJEnemyState::Dying && State != EJJEnemyState::Dead; }
	bool IsBoss() const;

private:
	void BuildBody();
	bool CanSee(const APawn* Player) const;
	void FaceTowards(const FVector& Where);
	void StartAttack(bool bClose);
	void DoAttack(APawn* Player, float Distance);
	void Die();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Head;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Eyes;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> HornL;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> HornR;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> Glow;

	EJJEnemyState State = EJJEnemyState::Idle;
	float Health = 20.f;
	float Cooldown = 1.f;
	float StateTime = 0.f;
	float FireAt = 0.f;
	float SightTimer = 0.f;
	float AnimTime = 0.f;
	float Seed = 0.f;
	bool bSeesPlayer = false;
	bool bFired = false;
	bool bMelee = false;
	FVector ChargeDir = FVector::ZeroVector;
	FVector DeathStart = FVector::ZeroVector;
	float DeathYaw = 0.f;
};
