#include "MyLobbyGameMode.h"
#include "MyLobbyHUD.h"
#include "GameFramework/GameStateBase.h"

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
