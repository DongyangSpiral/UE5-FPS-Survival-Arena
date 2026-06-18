#include "MyMenuGameMode.h"
#include "MyMainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void AMyMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!NewPlayer || !NewPlayer->IsLocalPlayerController())
		return;

	UMyMainMenuWidget* Menu = CreateWidget<UMyMainMenuWidget>(NewPlayer, UMyMainMenuWidget::StaticClass());
	if (Menu)
	{
		Menu->AddToViewport();
		NewPlayer->SetShowMouseCursor(true);
		NewPlayer->SetInputMode(FInputModeUIOnly());
	}
}

void AMyMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalPlayerController())
			continue;

		UMyMainMenuWidget* Menu = CreateWidget<UMyMainMenuWidget>(PC, UMyMainMenuWidget::StaticClass());
		if (Menu)
		{
			Menu->AddToViewport();
			PC->SetShowMouseCursor(true);
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}
