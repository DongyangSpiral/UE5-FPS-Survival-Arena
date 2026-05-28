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
}

void AMyGameStateBase::OnPlayerVictory(AMyPlayerState* Winner)
{
	if (bGameFinished || !Winner)
		return;

	bGameFinished = true;
	WinnerName = Winner->GetPlayerName();
	Multicast_ShowVictory(WinnerName);
}

void AMyGameStateBase::Multicast_ShowVictory_Implementation(const FString& Name)
{
	BP_OnVictory(Name);
}
