#include "MyLobbyGameMode.h"
#include "MyLobbyHUD.h"
#include "MyGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

AMyLobbyGameMode::AMyLobbyGameMode()
{
	HUDClass = AMyLobbyHUD::StaticClass();
}

void AMyLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
		return;

	if (GetNumPlayers() >= 4)
	{
		ErrorMessage = TEXT("Server is full (max 4 players).");
	}
}

void AMyLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!HasAuthority() || !NewPlayer || !NewPlayer->PlayerState)
		return;

	if (NewPlayer == GetWorld()->GetFirstPlayerController())
	{
		if (UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
		{
			if (!GI->PendingPlayerName.IsEmpty())
				NewPlayer->PlayerState->SetPlayerName(GI->PendingPlayerName);
		}
	}
}
