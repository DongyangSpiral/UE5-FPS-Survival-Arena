#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyMainMenuWidget.generated.h"

class UEditableTextBox;
class UCanvasPanel;

UCLASS()
class FPS_API UMyMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void OnCreateRoom();

	UFUNCTION()
	void OnJoinRoom();

	UFUNCTION()
	void OnQuit();

	UFUNCTION()
	void OnConnect();

	UFUNCTION()
	void OnCancelJoin();

	void ShowJoinOverlay();
	void HideJoinOverlay();

	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> OverlayContainer;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> IPInputBox;
};
