#include "MyUIWidget.h"
#include "MyGameStateBase.h"
#include "Engine/World.h"

void UMyUIWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (AMyGameStateBase* GS = GetWorld()->GetGameState<AMyGameStateBase>())
	{
		BP_UpdateScore(GS->TeamScore, GS->TargetScore);
		BP_UpdateAliveCount(GS->AlivePlayerCount);

		if (GS->bGameFinished && !bGameFinishedHandled)
		{
			bGameFinishedHandled = true;
			if (GS->WinnerName.IsEmpty())
				BP_ShowDefeat();
			else
				BP_ShowVictory(GS->WinnerName);
		}
	}

	if (DeathCountdown > 0.0f)
	{
		DeathCountdown -= InDeltaTime;
		BP_ShowDeathOverlay(DeathCountdown);
	}
}

void UMyUIWidget::UpdateHealth(float LifePercent)
{
	BP_UpdateHealth(LifePercent);
}

void UMyUIWidget::UpdateBulletCount(int32 Bullets, int32 MagazineSize)
{
	BP_UpdateBulletCount(Bullets, MagazineSize);
}

void UMyUIWidget::UpdateBulletTier(int32 Tier)
{
	BP_UpdateBulletTier(Tier);
}

void UMyUIWidget::ShowDeathOverlay(float RespawnTime)
{
	DeathCountdown = RespawnTime;
	BP_ShowDeathOverlay(DeathCountdown);
}
