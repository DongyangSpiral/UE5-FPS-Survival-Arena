#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyLobbyWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UButton;

UCLASS()
class FPS_API UMyLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void OnStartGame();

	UFUNCTION()
	void OnExitLobby();

	void RefreshPlayerList();
	void UpdateIPDisplay();

	UPROPERTY()
	TObjectPtr<UVerticalBox> EntryList;

	UPROPERTY()
	TObjectPtr<UTextBlock> IPText;

	UPROPERTY()
	TObjectPtr<UButton> StartButton;

	UPROPERTY()
	TObjectPtr<UButton> ExitButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> WaitingText;

	float ListRefreshTimer = 0.f;
};
