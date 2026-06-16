#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

class AMyPlayerState;

UCLASS()
class FPS_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMyGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	FString WinnerName;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	bool bGameFinished = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	int32 TargetScore = 10;

	UPROPERTY(ReplicatedUsing = OnRep_AliveCount, BlueprintReadOnly, Category = "Score")
	int32 AlivePlayerCount = 0;

	void ResetState();
	void SetAlivePlayerCount(int32 NewCount);
	void OnPlayerVictory(AMyPlayerState* Winner);
	void FreezeAllActors();

	UFUNCTION()
	void OnRep_AliveCount();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowVictory(const FString& Name);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowDefeat();

protected:
	void CheckDefeatCondition();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Victory"))
	void BP_OnVictory(const FString& InWinnerName);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Defeat"))
	void BP_OnDefeat();
};
