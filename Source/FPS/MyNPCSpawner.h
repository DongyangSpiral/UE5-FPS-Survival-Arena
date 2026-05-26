#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AI/ShooterNPCSpawner.h"
#include "MyNPCSpawner.generated.h"

class AShooterNPC;
class AMyGameStateBase;

UCLASS()
class FPS_API AMyNPCSpawner : public AShooterNPCSpawner
{
	GENERATED_BODY()

public:
	AMyNPCSpawner();

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 MaxEnemyCount = 15;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = 1, ClampMax = 10, Units = "s"))
	float SpawnInterval = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = 0, Units = "cm"))
	float SpawnRadius = 300.0f;

protected:
	void SpawnTick();
	void SpawnOneNPC();
	void StopSpawning();

	UFUNCTION()
	void OnNPCDead();

	UPROPERTY()
	TArray<TObjectPtr<AShooterNPC>> AliveNPCs;

	UPROPERTY()
	TObjectPtr<AMyGameStateBase> MyGameState;

	FTimerHandle SpawnTimerHandle;
};
