#include "MyNetworkedNPC.h"
#include "FPS.h"
#include "MyPlayerState.h"
#include "MyGameStateBase.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AMyNetworkedNPC::AMyNetworkedNPC()
{
}

void AMyNetworkedNPC::BeginPlay()
{
	TSubclassOf<AShooterWeapon> SavedWeaponClass = WeaponClass;
	if (!HasAuthority())
		WeaponClass = nullptr;

	Super::BeginPlay();

	WeaponClass = SavedWeaponClass;

	if (HasAuthority() && Weapon)
		Weapon->SetReplicates(true);
}

float AMyNetworkedNPC::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float Result = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (HasAuthority())
	{
		ReplicatedCurrentHP = CurrentHP;
		ReplicatedbIsDead = bIsDead;
	}

	return Result;
}

void AMyNetworkedNPC::OnRep_CurrentHP()
{
	if (!HasAuthority())
		CurrentHP = ReplicatedCurrentHP;
}

void AMyNetworkedNPC::OnRep_bIsDead()
{
	if (!HasAuthority() && ReplicatedbIsDead)
	{
		bIsDead = true;
		Tags.Add(DeathTag);
		SafeStopShooting();

		if (Weapon)
			Weapon->DeactivateWeapon();

		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName(RagdollCollisionProfile);
		GetCharacterMovement()->StopMovementImmediately();
		SetLifeSpan(DeferredDestructionTime);
	}
}

void AMyNetworkedNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyNetworkedNPC, ReplicatedCurrentHP);
	DOREPLIFETIME(AMyNetworkedNPC, ReplicatedbIsDead);
}
