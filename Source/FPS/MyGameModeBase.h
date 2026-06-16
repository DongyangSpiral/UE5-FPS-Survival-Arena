#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

class UMyUIWidget;

UCLASS()
class FPS_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyGameModeBase();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void SyncAliveCount();
	void HandleCharacterDeath(AController* DeadPlayer);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMyUIWidget> MyUIClass;

	TSet<APlayerController*> AlivePlayers;
};
