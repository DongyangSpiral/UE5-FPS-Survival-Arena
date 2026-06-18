#include "MyLobbyHUD.h"
#include "MyLobbyWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void AMyLobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	LobbyWidget = CreateWidget<UMyLobbyWidget>(GetOwningPlayerController(), UMyLobbyWidget::StaticClass());
	if (LobbyWidget)
	{
		LobbyWidget->AddToViewport();

		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			PC->SetShowMouseCursor(true);
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}
