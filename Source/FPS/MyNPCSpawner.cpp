#include "MyNPCSpawner.h"
#include "FPS.h"
#include "MyGameStateBase.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

AMyNPCSpawner::AMyNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMyNPCSpawner::BeginPlay()
{
	Super::BeginPlay();

	MyGameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AMyNPCSpawner::SpawnTick, SpawnInterval, true);
		UE_LOG(LogFPS, Verbose, TEXT("AMyNPCSpawner::BeginPlay - Spawn tick started (interval=%.1f, max=%d, radius=%.0f)"),
			SpawnInterval, MaxEnemyCount, SpawnRadius);
	}
	else
	{
		UE_LOG(LogFPS, Warning, TEXT("AMyNPCSpawner::BeginPlay - Not authority, will NOT spawn NPCs"));
	}
}

void AMyNPCSpawner::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	StopSpawning();
}

void AMyNPCSpawner::SpawnTick()
{
	if (MyGameState && MyGameState->bGameFinished)
	{
		UE_LOG(LogFPS, Verbose, TEXT("AMyNPCSpawner::SpawnTick - Game finished, stopping spawn"));
		StopSpawning();
		return;
	}

	for (int32 i = AliveNPCs.Num() - 1; i >= 0; i--)
	{
		AShooterNPC* NPC = AliveNPCs[i];
		if (!IsValid(NPC) || NPC->CurrentHP <= 0.0f)
		{
			AliveNPCs.RemoveAt(i);
		}
	}

	int32 NumSpawned = 0;
	while (AliveNPCs.Num() < MaxEnemyCount)
	{
		SpawnOneNPC();
		NumSpawned++;
	}

	if (NumSpawned > 0)
	{
		UE_LOG(LogFPS, Verbose, TEXT("AMyNPCSpawner::SpawnTick - Spawned %d NPCs, total alive=%d/%d"), NumSpawned, AliveNPCs.Num(), MaxEnemyCount);
	}
}

void AMyNPCSpawner::SpawnOneNPC()
{
	if (!NPCClass)
	{
		UE_LOG(LogFPS, Error, TEXT("AMyNPCSpawner::SpawnOneNPC - NPCClass is NULL! Set NPCClass in BP_SurvivalNPCSpawner."));
		return;
	}

	float CapsuleHalfHeight = 90.0f;
	FVector SpawnLoc = GetActorLocation() + FVector(0.0f, 0.0f, CapsuleHalfHeight);
	SpawnLoc.X += FMath::FRandRange(-SpawnRadius, SpawnRadius);
	SpawnLoc.Y += FMath::FRandRange(-SpawnRadius, SpawnRadius);

	FTransform SpawnTransform(GetActorRotation(), SpawnLoc);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AShooterNPC* NPC = GetWorld()->SpawnActor<AShooterNPC>(NPCClass, SpawnTransform, SpawnParams);
	if (NPC)
	{
		NPC->SpawnDefaultController();
		AliveNPCs.Add(NPC);
		NPC->OnPawnDeath.AddDynamic(this, &AMyNPCSpawner::OnNPCDead);

		AAIController* AIC = NPC->GetController<AAIController>();
		UE_LOG(LogFPS, Verbose, TEXT("AMyNPCSpawner::SpawnOneNPC - Spawned NPC, controller=%s, alive count=%d"),
			AIC ? TEXT("OK") : TEXT("NULL"), AliveNPCs.Num());
	}
	else
	{
		UE_LOG(LogFPS, Error, TEXT("AMyNPCSpawner::SpawnOneNPC - SpawnActor failed!"));
	}
}

void AMyNPCSpawner::OnNPCDead()
{
	for (int32 i = AliveNPCs.Num() - 1; i >= 0; i--)
	{
		if (!IsValid(AliveNPCs[i]) || AliveNPCs[i]->CurrentHP <= 0.0f)
		{
			AliveNPCs.RemoveAt(i);
		}
	}
}

void AMyNPCSpawner::StopSpawning()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}
