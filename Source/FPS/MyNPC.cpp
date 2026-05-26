#include "MyNPC.h"
#include "FPS.h"
#include "MyGameStateBase.h"
#include "AIController.h"
#include "Variant_Shooter/AI/ShooterAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AMyNPC::AMyNPC()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMyNPC::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (!Weapon)
		{
			UE_LOG(LogFPS, Error, TEXT("AMyNPC::BeginPlay - Weapon is NULL after Super::BeginPlay! Check WeaponClass in BP_SurvivalNPC."));
		}

		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AMyNPC::StopStateTree);
		GetWorld()->GetTimerManager().SetTimer(ChaseTimerHandle, this, &AMyNPC::ChaseTick, ChaseInterval, true);

		UE_LOG(LogFPS, Verbose, TEXT("AMyNPC::BeginPlay - ChaseTick started (interval=%.1f)"), ChaseInterval);
	}
}

void AMyNPC::StopStateTree()
{
	if (AShooterAIController* AIC = Cast<AShooterAIController>(GetController()))
	{
		if (UStateTreeAIComponent* ST = AIC->FindComponentByClass<UStateTreeAIComponent>())
		{
			ST->StopLogic("CustomChase");
			UE_LOG(LogFPS, Verbose, TEXT("AMyNPC::StopStateTree - StateTree stopped"));
		}
	}
	else
	{
		UE_LOG(LogFPS, Warning, TEXT("AMyNPC::StopStateTree - Controller is NULL or not AShooterAIController"));
	}
}

void AMyNPC::SafeStopShooting()
{
	if (Weapon)
	{
		StopShooting();
	}
}

void AMyNPC::SafeStartShooting(AActor* ActorToShoot)
{
	if (Weapon && ActorToShoot)
	{
		StartShooting(ActorToShoot);
	}
}

void AMyNPC::ChaseTick()
{
	if (bIsDead)
		return;

	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Player"), Players);

	if (Players.Num() == 0)
	{
		UE_LOG(LogFPS, Warning, TEXT("AMyNPC::ChaseTick - No actors with 'Player' tag found!"));
		SafeStopShooting();
		return;
	}

	AActor* NearestPlayer = nullptr;
	float NearestDist = FLT_MAX;

	for (AActor* Player : Players)
	{
		float Dist = FVector::Dist2D(GetActorLocation(), Player->GetActorLocation());
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			NearestPlayer = Player;
		}
	}

	if (NearestPlayer)
	{
		AAIController* AIC = Cast<AAIController>(GetController());
		if (AIC)
		{
			if (NearestDist > AttackRange)
			{
				UE_LOG(LogFPS, Verbose, TEXT("ChaseTick - Chasing player dist=%.0f > range=%.0f"), NearestDist, AttackRange);
				AIC->MoveToActor(NearestPlayer, AttackRange * 0.5f, true, true, false, 0, true);
				SafeStopShooting();
			}
			else
			{
				UE_LOG(LogFPS, Verbose, TEXT("ChaseTick - Attacking player dist=%.0f <= range=%.0f"), NearestDist, AttackRange);
				AIC->MoveToActor(NearestPlayer, AttackRange * 0.3f, true, true, false, 0, true);
				SafeStartShooting(NearestPlayer);
			}
		}
		else
		{
			UE_LOG(LogFPS, Error, TEXT("AMyNPC::ChaseTick - Controller is NULL! NPC cannot move."));
		}
	}
	else
	{
		SafeStopShooting();
	}
}

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
		SafeStopShooting();

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
