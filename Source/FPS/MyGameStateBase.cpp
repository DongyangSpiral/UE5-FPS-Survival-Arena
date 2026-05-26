#include "MyGameStateBase.h"
#include "Net/UnrealNetwork.h"

AMyGameStateBase::AMyGameStateBase()
{
}

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameStateBase, TeamScore);
	DOREPLIFETIME(AMyGameStateBase, AlivePlayerCount);
	DOREPLIFETIME(AMyGameStateBase, bGameFinished);
}

void AMyGameStateBase::AddScore(int32 Amount)
{
	if (bGameFinished)
		return;

	TeamScore += Amount;
	OnTeamScoreChangedDelegate.Broadcast(TeamScore);

	if (TeamScore >= TargetScore)
	{
		bGameFinished = true;
		Multicast_ShowVictory();
	}
}

void AMyGameStateBase::OnPlayerDied()
{
	AlivePlayerCount = FMath::Max(0, AlivePlayerCount - 1);
	OnAliveCountChangedDelegate.Broadcast(AlivePlayerCount);

	if (AlivePlayerCount <= 0 && !bGameFinished)
	{
		bGameFinished = true;
		Multicast_ShowGameOver();
	}
}

void AMyGameStateBase::OnPlayerRespawned()
{
	AlivePlayerCount++;
	OnAliveCountChangedDelegate.Broadcast(AlivePlayerCount);
}

void AMyGameStateBase::Multicast_ShowVictory_Implementation()
{
	BP_OnVictory();
}

void AMyGameStateBase::Multicast_ShowGameOver_Implementation()
{
	BP_OnGameOver();
}

void AMyGameStateBase::OnRep_TeamScore()
{
	OnTeamScoreChangedDelegate.Broadcast(TeamScore);
}

void AMyGameStateBase::OnRep_AlivePlayerCount()
{
	OnAliveCountChangedDelegate.Broadcast(AlivePlayerCount);
}
