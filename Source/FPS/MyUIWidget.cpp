#include "MyUIWidget.h"
#include "MyCharacter.h"
#include "MyGameStateBase.h"
#include "MyPlayerState.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UMyUIWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APawn* CurrentPawn = GetOwningPlayerPawn();

	// Auto-bind when pawn changes (initial spawn or respawn)
	if (CurrentPawn && CurrentPawn != CachedPawn)
	{
		BindToPawn(Cast<AShooterCharacter>(CurrentPawn));
		DeathCountdown = -1.0f;
		BP_HideDeathOverlay();
	}
	else if (!CurrentPawn && CachedPawn)
	{
		UnbindFromPawn();
	}

	// Detect death via IsDead()
	if (AMyCharacter* MyChar = Cast<AMyCharacter>(CurrentPawn))
	{
		if (MyChar->IsDead() && DeathCountdown < 0.0f)
		{
			DeathCountdown = MyChar->GetRespawnTime();
			BP_ShowDeathOverlay(DeathCountdown);
		}
	}

	// Tick death countdown
	if (DeathCountdown > 0.0f)
	{
		DeathCountdown -= InDeltaTime;
		BP_ShowDeathOverlay(FMath::Max(0.0f, DeathCountdown));
	}

	// Poll GameState
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

	// Poll bullet tier from PlayerState
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>())
		{
			BP_UpdateBulletTier(PS->WeaponTier);
		}
	}
}

void UMyUIWidget::BindToPawn(AShooterCharacter* NewPawn)
{
	UnbindFromPawn();

	CachedPawn = NewPawn;
	if (!NewPawn)
		return;

	NewPawn->OnBulletCountUpdated.AddDynamic(this, &UMyUIWidget::OnBulletCountChanged);
	NewPawn->OnDamaged.AddDynamic(this, &UMyUIWidget::OnPawnDamaged);
	NewPawn->OnDestroyed.AddDynamic(this, &UMyUIWidget::OnPawnDestroyed);

	if (AMyCharacter* MyChar = Cast<AMyCharacter>(NewPawn))
	{
		MyChar->OnBulletTierChanged.AddDynamic(this, &UMyUIWidget::OnBulletTierChanged);

		BP_UpdateHealth(MyChar->GetLifePercent());

		if (AShooterWeapon* Weapon = MyChar->GetCurrentWeapon())
		{
			OnBulletCountChanged(Weapon->GetMagazineSize(), Weapon->GetBulletCount());
		}
	}
}

void UMyUIWidget::UnbindFromPawn()
{
	if (CachedPawn)
	{
		CachedPawn->OnBulletCountUpdated.RemoveAll(this);
		CachedPawn->OnDamaged.RemoveAll(this);
		CachedPawn->OnDestroyed.RemoveAll(this);

		if (AMyCharacter* MyChar = Cast<AMyCharacter>(CachedPawn.Get()))
		{
			MyChar->OnBulletTierChanged.RemoveAll(this);
		}
	}

	CachedPawn = nullptr;
}

void UMyUIWidget::OnBulletCountChanged(int32 MagazineSize, int32 Bullets)
{
	BP_UpdateBulletCount(Bullets, MagazineSize);
}

void UMyUIWidget::OnPawnDamaged(float LifePercent)
{
	BP_UpdateHealth(LifePercent);
}

void UMyUIWidget::OnPawnDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == CachedPawn)
	{
		CachedPawn = nullptr;
	}
}

void UMyUIWidget::OnBulletTierChanged(int32 NewTier)
{
	BP_UpdateBulletTier(NewTier);
	BP_ShowTierUp(NewTier);
}
