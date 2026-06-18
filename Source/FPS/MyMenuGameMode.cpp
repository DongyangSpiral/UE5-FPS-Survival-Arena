#include "MyMenuGameMode.h"
#include "MyMainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void AMyMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	UMyMainMenuWidget* Menu = CreateWidget<UMyMainMenuWidget>(GetWorld(), UMyMainMenuWidget::StaticClass());
	if (Menu)
	{
		Menu->AddToViewport();
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());
	}
}
