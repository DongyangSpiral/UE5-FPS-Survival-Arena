#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "MyCharacter.generated.h"

class AMyGameStateBase;
class AShooterWeapon;
class UInputMappingContext;

UCLASS()
class FPS_API AMyCharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AShooterWeapon> DefaultWeaponClass;

protected:
	void OnRespawn();
	void GiveWeapon();

	UPROPERTY()
	TSubclassOf<AShooterWeapon> FallbackWeaponClass;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> WeaponIMC;

	AMyGameStateBase* MyGameState;
};
