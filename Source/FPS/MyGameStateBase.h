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

	void OnPlayerVictory(AMyPlayerState* Winner);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowVictory(const FString& Name);

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Victory"))
	void BP_OnVictory(const FString& InWinnerName);
};
