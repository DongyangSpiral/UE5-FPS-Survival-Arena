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
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedWeapon)
	TObjectPtr<AShooterWeapon> ReplicatedWeapon;

	UFUNCTION()
	void OnRep_ReplicatedWeapon();

	void ApplyReplicatedWeapon();

protected:
	void OnRespawn();
	void GiveWeapon();

	UFUNCTION(Server, Reliable) void Server_StartFiring(FVector_NetQuantize AimTarget);
	UFUNCTION(Server, Reliable) void Server_StopFiring();

	void Input_StartFiring();
	void Input_StopFiring();
	void Input_SwitchWeapon();

	UPROPERTY()
	TSubclassOf<AShooterWeapon> FallbackWeaponClass;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> WeaponIMC;
};
