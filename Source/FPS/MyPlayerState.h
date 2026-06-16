#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

class UMyUIWidget;
class AMyGameStateBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScoreChangedDelegate, int32, NewScore, int32, TargetScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponTierChangedDelegate, int32, NewTier);

UCLASS()
class FPS_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMyPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponTier, BlueprintReadOnly)
	int32 WeaponTier = 0;

	UPROPERTY()
	TObjectPtr<UMyUIWidget> MyUIWidget;

	UPROPERTY(BlueprintAssignable)
	FScoreChangedDelegate OnScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FWeaponTierChangedDelegate OnWeaponTierChanged;

	void AddScore(int32 Amount);
	void ResetScore();

	UFUNCTION()
	void OnRep_WeaponTier();

	UFUNCTION()
	virtual void OnRep_Score();

	int32 GetTierForScore(int32 InScore) const;
};
