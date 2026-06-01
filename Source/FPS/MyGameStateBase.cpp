#include "MyGameStateBase.h"
#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"

AMyGameStateBase::AMyGameStateBase()
{
}

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameStateBase, WinnerName);
	DOREPLIFETIME(AMyGameStateBase, bGameFinished);
	DOREPLIFETIME(AMyGameStateBase, TeamScore);
	DOREPLIFETIME(AMyGameStateBase, AlivePlayerCount);
}

void AMyGameStateBase::AddScore(int32 Amount)
{
	if (!HasAuthority())
		return;

	TeamScore += Amount;
	OnRep_TeamScore();

	if (TeamScore >= TargetScore && !bGameFinished)
	{
		bGameFinished = true;

		AMyPlayerState* BestPlayer = nullptr;
		float BestScore = -1.0f;
		for (APlayerState* PS : PlayerArray)
		{
			if (AMyPlayerState* MPS = Cast<AMyPlayerState>(PS))
			{
				if (MPS->GetScore() > BestScore)
				{
					BestScore = MPS->GetScore();
					BestPlayer = MPS;
				}
			}
		}

		WinnerName = BestPlayer ? BestPlayer->GetPlayerName() : TEXT("Team");
		Multicast_ShowVictory(WinnerName);
	}
}

void AMyGameStateBase::OnPlayerDied()
{
	if (!HasAuthority())
		return;

	AlivePlayerCount = FMath::Max(0, AlivePlayerCount - 1);
	OnRep_AliveCount();

	if (AlivePlayerCount <= 0 && !bGameFinished)
	{
		bGameFinished = true;
		WinnerName.Empty();
		Multicast_ShowDefeat();
	}
}

void AMyGameStateBase::OnPlayerRespawned()
{
	if (!HasAuthority())
		return;

	++AlivePlayerCount;
	OnRep_AliveCount();
}

void AMyGameStateBase::OnPlayerVictory(AMyPlayerState* Winner)
{
	if (bGameFinished || !Winner)
		return;

	bGameFinished = true;
	WinnerName = Winner->GetPlayerName();
	TeamScore = TargetScore;
	OnRep_TeamScore();
	Multicast_ShowVictory(WinnerName);
}

void AMyGameStateBase::OnRep_TeamScore()
{
}

void AMyGameStateBase::OnRep_AliveCount()
{
}

void AMyGameStateBase::Multicast_ShowVictory_Implementation(const FString& Name)
{
	BP_OnVictory(Name);
}

void AMyGameStateBase::Multicast_ShowDefeat_Implementation()
{
	BP_OnDefeat();
}
