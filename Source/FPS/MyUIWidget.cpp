#include "MyUIWidget.h"
#include "MyCharacter.h"
#include "MyGameStateBase.h"
#include "MyPlayerState.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UMyUIWidget::NativeConstruct()
{
	Super::NativeConstruct();
	TryBindGameState();
	TryBindPlayerState();
}

void UMyUIWidget::NativeDestruct()
{
	UnbindFromPawn();

	if (CachedGameState)
	{
		CachedGameState->OnAliveCountChanged.RemoveAll(this);
		CachedGameState->OnGameFinished.RemoveAll(this);
	}
	if (CachedPlayerState)
	{
		CachedPlayerState->OnScoreChanged.RemoveAll(this);
		CachedPlayerState->OnWeaponTierChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UMyUIWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APawn* CurrentPawn = GetOwningPlayerPawn();

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

	if (DeathCountdown > 0.0f)
	{
		DeathCountdown -= InDeltaTime;
		BP_ShowDeathOverlay(FMath::Max(0.0f, DeathCountdown));
	}

	LeaderboardRefreshTimer -= InDeltaTime;
	if (LeaderboardRefreshTimer <= 0.0f)
	{
		UpdateLeaderboard();
		LeaderboardRefreshTimer = 0.3f;
	}
}

void UMyUIWidget::TryBindGameState()
{
	if (CachedGameState)
		return;

	CachedGameState = GetWorld()->GetGameState<AMyGameStateBase>();
	if (!CachedGameState)
	{
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &UMyUIWidget::TryBindGameState), 0.1f, false);
		return;
	}

	CachedGameState->OnAliveCountChanged.AddDynamic(this, &UMyUIWidget::OnAliveCountChanged);
	CachedGameState->OnGameFinished.AddDynamic(this, &UMyUIWidget::OnGameFinished);
	BP_UpdateAliveCount(CachedGameState->AlivePlayerCount);
}

void UMyUIWidget::TryBindPlayerState()
{
	if (CachedPlayerState)
		return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &UMyUIWidget::TryBindPlayerState), 0.1f, false);
		return;
	}

	CachedPlayerState = PC->GetPlayerState<AMyPlayerState>();
	if (!CachedPlayerState)
	{
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &UMyUIWidget::TryBindPlayerState), 0.1f, false);
		return;
	}

	CachedPlayerState->OnScoreChanged.AddDynamic(this, &UMyUIWidget::OnScoreChanged);
	CachedPlayerState->OnWeaponTierChanged.AddDynamic(this, &UMyUIWidget::OnWeaponTierChanged);

	if (AMyGameStateBase* GS = GetWorld()->GetGameState<AMyGameStateBase>())
		OnScoreChanged(FMath::RoundToInt(CachedPlayerState->GetScore()), GS->TargetScore);

	OnWeaponTierChanged(CachedPlayerState->WeaponTier);
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
		MyChar->OnMyCharacterDied.AddDynamic(this, &UMyUIWidget::OnCharacterDied);

		BP_UpdateHealth(MyChar->GetLifePercent());

		if (AShooterWeapon* Weapon = MyChar->GetCurrentWeapon())
			OnBulletCountChanged(Weapon->GetMagazineSize(), Weapon->GetBulletCount());
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
			MyChar->OnMyCharacterDied.RemoveAll(this);
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
		CachedPawn = nullptr;
}

void UMyUIWidget::OnBulletTierChanged(int32 NewTier)
{
	BP_UpdateBulletTier(NewTier);
	BP_ShowTierUp(NewTier);
}

void UMyUIWidget::OnCharacterDied(float RespawnTime)
{
	if (CachedGameState && CachedGameState->bGameFinished)
		return;

	DeathCountdown = RespawnTime;
	BP_ShowDeathOverlay(DeathCountdown);
}

void UMyUIWidget::OnScoreChanged(int32 NewScore, int32 TargetScore)
{
	BP_UpdateScore(NewScore, TargetScore);
}

void UMyUIWidget::OnWeaponTierChanged(int32 NewTier)
{
	BP_UpdateBulletTier(NewTier);
}

void UMyUIWidget::OnAliveCountChanged(int32 NewCount)
{
	BP_UpdateAliveCount(NewCount);
}

void UMyUIWidget::OnGameFinished(const FString& WinnerName, bool bIsVictory)
{
	if (bGameFinishedHandled)
		return;

	bGameFinishedHandled = true;
	DeathCountdown = -1.0f;
	BP_HideDeathOverlay();

	if (bIsVictory)
		BP_ShowVictory(WinnerName);
	else
		BP_ShowDefeat();
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

	Entries.Sort([](const FLeaderboardEntry& A, const FLeaderboardEntry& B)
	{
		return A.Score > B.Score;
	});

	AMyGameStateBase* MGS = Cast<AMyGameStateBase>(GS);
	int32 Target = MGS ? MGS->TargetScore : 10;

	BP_UpdateLeaderboard(Entries, Target);
}
