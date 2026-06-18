#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "MyNPC.generated.h"

class AMyGameStateBase;

UCLASS()
class FPS_API AMyNPC : public AShooterNPC
{
	GENERATED_BODY()

public:
	AMyNPC();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackRange = 600.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float ChaseInterval = 0.3f;

protected:
	void ChaseTick();
	void StopStateTree();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SafeStopShooting();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SafeStartShooting(AActor* ActorToShoot);

	FTimerHandle ChaseTimerHandle;

	UPROPERTY()
	TObjectPtr<AMyGameStateBase> MyGameState;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float ReplicatedCurrentHP = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_bIsDead)
	bool ReplicatedbIsDead = false;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedWeapon)
	TObjectPtr<AShooterWeapon> ReplicatedWeapon;

	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_bIsDead();

	UFUNCTION()
	void OnRep_ReplicatedWeapon();

	void ApplyDeathState();
};
