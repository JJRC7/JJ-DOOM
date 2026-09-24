#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JJTypes.h"
#include "JJCharacter.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class USpotLightComponent;

// Jugador en primera persona: movimiento, armas, linterna, recoger objetos y usar puertas.
UCLASS()
class JJDOOM_API AJJCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AJJCharacter();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	bool TryPickup(EJJPickup Type, int32 Amount, bool bCounted);
	bool SelectWeapon(EJJWeapon Weapon);
	void Revive();
	bool IsDead() const { return bDead; }

	FJJInventory Inv;
	bool bRedKey = false;
	bool bBlueKey = false;
	float DamageFlash = 0.f;
	float PickupFlash = 0.f;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void TurnRate(float Value);
	void LookUpRate(float Value);
	void StartFire();
	void StopFire();
	void Use();
	void SprintOn();
	void SprintOff();
	void ToggleFlashlight();
	void Continue();
	void Weapon1();
	void Weapon2();
	void Weapon3();
	void Weapon4();
	void NextWeapon();
	void PrevWeapon();
	void CycleWeapon(int32 Step);

	void FireWeapon();
	void AutoSwitch();
	void UpdateWeaponModel();
	bool CanAct() const;

	UPROPERTY(VisibleAnywhere, Category = "JJ")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> GunBody;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> GunBarrel;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> GunHand;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> MuzzleLight;

	UPROPERTY()
	TObjectPtr<USpotLightComponent> Flashlight;

	FVector BodyBase = FVector::ZeroVector;
	FVector BarrelBase = FVector::ZeroVector;
	FVector HandBase = FVector::ZeroVector;
	bool bDead = false;
	bool bFiring = false;
	float Cooldown = 0.f;
	float MuzzleTime = 0.f;
	float BobTime = 0.f;
	float SwitchTime = 0.f;
	float DeadTime = 0.f;
};
