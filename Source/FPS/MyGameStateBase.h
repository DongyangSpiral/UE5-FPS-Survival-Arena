#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamScoreChanged, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAliveCountChanged, int32, NewCount);

UCLASS()
class FPS_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMyGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_TeamScore, BlueprintReadOnly, Category = "Score")
	int32 TeamScore = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 TargetScore = 10;

	UPROPERTY(ReplicatedUsing = OnRep_AlivePlayerCount, BlueprintReadOnly, Category = "Game")
	int32 AlivePlayerCount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	bool bGameFinished = false;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTeamScoreChanged OnTeamScoreChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAliveCountChanged OnAliveCountChangedDelegate;

	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Amount);

	void OnPlayerDied();
	void OnPlayerRespawned();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowVictory();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowGameOver();

protected:
	UFUNCTION()
	void OnRep_TeamScore();

	UFUNCTION()
	void OnRep_AlivePlayerCount();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Victory"))
	void BP_OnVictory();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Game Over"))
	void BP_OnGameOver();
};
