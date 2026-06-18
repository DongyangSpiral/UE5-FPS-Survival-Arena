#include "MyLobbyGameMode.h"
#include "MyLobbyHUD.h"

AMyLobbyGameMode::AMyLobbyGameMode()
{
	HUDClass = AMyLobbyHUD::StaticClass();
}
