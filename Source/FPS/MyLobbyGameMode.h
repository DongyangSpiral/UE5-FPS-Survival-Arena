#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyLobbyGameMode.generated.h"

UCLASS()
class FPS_API AMyLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyLobbyGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
