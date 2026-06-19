#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyMainMenuWidget.generated.h"

class UEditableTextBox;
class UCanvasPanel;
class UButton;
class UTextBlock;
class UTexture2D;

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

	FString GetSanitizedPlayerName() const;

	void ShowJoinOverlay();
	void HideJoinOverlay();

	UPROPERTY(EditDefaultsOnly, Category = "Background")
	TObjectPtr<UTexture2D> BackgroundTexture;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> OverlayContainer;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> IPInputBox;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> NameInputBox;

	UPROPERTY()
	TObjectPtr<UButton> CreateBtn;

	UPROPERTY()
	TObjectPtr<UButton> JoinBtn;

	UPROPERTY()
	TObjectPtr<UButton> QuitBtn;

	UPROPERTY()
	TObjectPtr<UTextBlock> WaitingText;
};
