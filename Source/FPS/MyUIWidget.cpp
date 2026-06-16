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

	// Step 1: Poll GameState FIRST (victory/defeat takes priority)
	if (AMyGameStateBase* GS = GetWorld()->GetGameState<AMyGameStateBase>())
	{
		int32 MyScore = 0;
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>())
				MyScore = FMath::RoundToInt(PS->GetScore());
		}
		BP_UpdateScore(MyScore, GS->TargetScore);
		BP_UpdateAliveCount(GS->AlivePlayerCount);

		if (GS->bGameFinished && !bGameFinishedHandled)
		{
			bGameFinishedHandled = true;
			DeathCountdown = -1.0f;
			BP_HideDeathOverlay();
			if (GS->WinnerName.IsEmpty())
				BP_ShowDefeat();
			else
				BP_ShowVictory(GS->WinnerName);
		}
	}

	// Step 2: Death detection
	if (AMyCharacter* MyChar = Cast<AMyCharacter>(CurrentPawn))
	{
		if (MyChar->IsDead() && DeathCountdown < 0.0f)
		{
			AMyGameStateBase* GS = GetWorld()->GetGameState<AMyGameStateBase>();
			if (!GS || !GS->bGameFinished)
			{
				DeathCountdown = MyChar->GetRespawnTime();
				BP_ShowDeathOverlay(DeathCountdown);
			}
		}
	}

	// Step 3: Tick death countdown
	if (DeathCountdown > 0.0f)
	{
		DeathCountdown -= InDeltaTime;
		AMyGameStateBase* GS = GetWorld()->GetGameState<AMyGameStateBase>();
		if (!GS || !GS->bGameFinished)
		{
			BP_ShowDeathOverlay(FMath::Max(0.0f, DeathCountdown));
		}
	}

	// Step 4: Poll bullet tier from PlayerState
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>())
		{
			BP_UpdateBulletTier(PS->WeaponTier);
		}
	}

	// Refresh leaderboard every 0.3s (avoid sorting every frame)
	LeaderboardRefreshTimer -= InDeltaTime;
	if (LeaderboardRefreshTimer <= 0.0f)
	{
		UpdateLeaderboard();
		LeaderboardRefreshTimer = 0.3f;
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

void UMyUIWidget::UpdateLeaderboard()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	APlayerController* LocalPC = GetOwningPlayer();
	if (!LocalPC)
		return;

	AGameStateBase* GS = World->GetGameState();
	if (!GS)
		return;

	TArray<FLeaderboardEntry> Entries;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS)
			continue;

		FLeaderboardEntry Entry;
		Entry.PlayerName = PS->GetPlayerName();
		Entry.Score = FMath::RoundToInt(PS->GetScore());
		Entry.bIsLocalPlayer = (PS == LocalPC->PlayerState);
		Entries.Add(Entry);
	}

	// Sort descending by Score
	Entries.Sort([](const FLeaderboardEntry& A, const FLeaderboardEntry& B)
	{
		return A.Score > B.Score;
	});

	AMyGameStateBase* MGS = Cast<AMyGameStateBase>(GS);
	int32 Target = MGS ? MGS->TargetScore : 10;

	BP_UpdateLeaderboard(Entries, Target);
}
