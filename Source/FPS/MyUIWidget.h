#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUIWidget.generated.h"

class AShooterCharacter;
class AMyCharacter;
class AMyGameStateBase;

USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLocalPlayer = false;
};

UCLASS()
class FPS_API UMyUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void BindToPawn(AShooterCharacter* NewPawn);
	void UnbindFromPawn();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateHealth(float LifePercent);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateBulletCount(int32 Bullets, int32 MagazineSize);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateScore(int32 TeamScore, int32 TargetScore);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateAliveCount(int32 Count);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateBulletTier(int32 Tier);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowDeathOverlay(float RespawnTime);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_HideDeathOverlay();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowVictory(const FString& WinnerName);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowDefeat();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowTierUp(int32 NewTier);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateLeaderboard(const TArray<FLeaderboardEntry>& Entries, int32 TargetScore);

private:
	void UpdateLeaderboard();

	UFUNCTION()
	void OnBulletCountChanged(int32 MagazineSize, int32 Bullets);

	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnBulletTierChanged(int32 NewTier);

	UPROPERTY()
	TObjectPtr<AShooterCharacter> CachedPawn;

	bool bGameFinishedHandled = false;
	float DeathCountdown = -1.0f;
	float LeaderboardRefreshTimer = 0.0f;
};
