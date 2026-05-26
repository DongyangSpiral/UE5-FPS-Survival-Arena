#include "MyNPCSpawner.h"
#include "MyGameStateBase.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

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

	while (AliveNPCs.Num() < MaxEnemyCount)
	{
		SpawnOneNPC();
	}
}

void AMyNPCSpawner::SpawnOneNPC()
{
	if (!NPCClass)
		return;

	FVector Origin = GetActorLocation();
	FVector RandomPos = Origin + FVector(
		FMath::FRandRange(-SpawnRadius, SpawnRadius),
		FMath::FRandRange(-SpawnRadius, SpawnRadius),
		0.0f
	);

	FTransform SpawnTransform(GetActorRotation(), RandomPos);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AShooterNPC* NPC = GetWorld()->SpawnActor<AShooterNPC>(NPCClass, SpawnTransform, SpawnParams);
	if (NPC)
	{
		AliveNPCs.Add(NPC);
		NPC->OnPawnDeath.AddDynamic(this, &AMyNPCSpawner::OnNPCDead);
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
