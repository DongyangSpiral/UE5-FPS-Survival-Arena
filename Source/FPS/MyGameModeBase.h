#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

class AMyGameStateBase;

UCLASS()
class FPS_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyGameModeBase();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	UPROPERTY()
	TObjectPtr<AMyGameStateBase> MyGameState;

	void InitAlivePlayerCount();
};
