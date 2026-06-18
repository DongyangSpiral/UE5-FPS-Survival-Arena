#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyLobbyHUD.generated.h"

class UMyLobbyWidget;

UCLASS()
class FPS_API AMyLobbyHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UMyLobbyWidget> LobbyWidget;
};
