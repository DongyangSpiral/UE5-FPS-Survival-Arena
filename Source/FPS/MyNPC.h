#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "MyNPC.generated.h"

UCLASS()
class FPS_API AMyNPC : public AShooterNPC
{
	GENERATED_BODY()

public:
	AMyNPC();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

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
};
