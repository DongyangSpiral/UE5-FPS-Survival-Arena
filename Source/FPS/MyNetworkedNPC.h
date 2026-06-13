#pragma once

#include "CoreMinimal.h"
#include "MyNPC.h"
#include "MyNetworkedNPC.generated.h"

UCLASS()
class FPS_API AMyNetworkedNPC : public AMyNPC
{
	GENERATED_BODY()

public:
	AMyNetworkedNPC();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float ReplicatedCurrentHP = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_bIsDead)
	bool ReplicatedbIsDead = false;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedWeapon)
	TObjectPtr<AShooterWeapon> ReplicatedWeapon;

	UFUNCTION()
	void OnRep_ReplicatedWeapon();

	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_bIsDead();
};
