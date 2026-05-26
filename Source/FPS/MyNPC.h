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
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
