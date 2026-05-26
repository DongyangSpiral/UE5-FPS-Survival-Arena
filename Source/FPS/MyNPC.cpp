#include "MyNPC.h"
#include "MyGameStateBase.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

float AMyNPC::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || CurrentHP <= 0.0f)
		return 0.0f;

	CurrentHP -= Damage;

	if (CurrentHP <= 0.0f)
	{
		if (AMyGameStateBase* GS = Cast<AMyGameStateBase>(GetWorld()->GetGameState()))
		{
			GS->AddScore(1);
		}

		bIsDead = true;
		Tags.Add(DeathTag);
		StopShooting();

		if (Weapon)
			Weapon->DeactivateWeapon();

		OnPawnDeath.Broadcast();
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName(RagdollCollisionProfile);
		GetCharacterMovement()->StopMovementImmediately();
		SetLifeSpan(DeferredDestructionTime);
	}

	return Damage;
}
