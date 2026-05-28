#include "SurvivalWeapon.h"
#include "Variant_Shooter/Weapons/ShooterProjectile.h"

void ASurvivalWeapon::SetTierProjectile(TSubclassOf<AShooterProjectile> NewClass)
{
	ProjectileClass = NewClass;
}
