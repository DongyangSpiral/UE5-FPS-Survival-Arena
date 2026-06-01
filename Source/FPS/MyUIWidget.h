#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUIWidget.generated.h"

class AMyGameStateBase;

UCLASS()
class FPS_API UMyUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void UpdateHealth(float LifePercent);
	void UpdateBulletCount(int32 Bullets, int32 MagazineSize);
	void UpdateBulletTier(int32 Tier);
	void ShowDeathOverlay(float RespawnTime);

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

private:
	bool bGameFinishedHandled = false;
	float DeathCountdown = -1.0f;
};
