#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "SurvivalWeapon.generated.h"

class AShooterProjectile;

UCLASS()
class FPS_API ASurvivalWeapon : public AShooterWeapon
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetTierProjectile(TSubclassOf<AShooterProjectile> NewClass);
};
