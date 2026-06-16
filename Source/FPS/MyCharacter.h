#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "MyCharacter.generated.h"

class AShooterWeapon;
class AShooterProjectile;
class UInputMappingContext;
class UMyUIWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyCharacterDiedDelegate, float, RespawnTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyBulletTierChangedDelegate, int32, NewTier);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterDeath, AController*);

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

	UPROPERTY(EditDefaultsOnly, Category = "Bullet Tiers")
	TSubclassOf<AShooterProjectile> TierProjectileClasses[4];

	void UpdateBulletTier(int32 NewTier);

	UFUNCTION(BlueprintPure)
	AShooterWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintPure)
	float GetLifePercent() const { return MaxHP > 0.0f ? FMath::Clamp(CurrentHP / MaxHP, 0.0f, 1.0f) : 0.0f; }

	UFUNCTION(BlueprintPure)
	float GetRespawnTime() const { return RespawnTime; }

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMyUIWidget> MyWidgetClass;

	UPROPERTY(BlueprintAssignable)
	FMyCharacterDiedDelegate OnMyCharacterDied;

	UPROPERTY(BlueprintAssignable)
	FMyBulletTierChangedDelegate OnBulletTierChanged;

	FOnCharacterDeath OnCharacterDeath;

protected:
	void OnRespawn();
	void GiveWeapon();

	UPROPERTY()
	TSubclassOf<AShooterWeapon> FallbackWeaponClass;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> WeaponIMC;
};
