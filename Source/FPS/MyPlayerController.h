#pragma once

#include "CoreMinimal.h"
#include "FPSPlayerController.h"
#include "MyPlayerController.generated.h"

class AShooterCharacter;
class UMyUIWidget;

UCLASS()
class FPS_API AMyPlayerController : public AFPSPlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	UFUNCTION()
	void OnMyCharacterDied(float RespawnTime);

	UFUNCTION()
	void OnBulletTierChanged(int32 NewTier);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMyUIWidget> MyUIClass;

	UPROPERTY()
	TObjectPtr<UMyUIWidget> MyUI;
};
