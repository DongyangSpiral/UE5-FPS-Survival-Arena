#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

class UMyUIWidget;

UCLASS()
class FPS_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMyPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponTier, BlueprintReadOnly)
	int32 WeaponTier = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	bool bPendingRespawn = false;

	UPROPERTY()
	TObjectPtr<UMyUIWidget> MyUIWidget;

	void AddScore(int32 Amount);
	void ResetScore();

	UFUNCTION()
	void OnRep_WeaponTier();

	int32 GetTierForScore(int32 InScore) const;
};
