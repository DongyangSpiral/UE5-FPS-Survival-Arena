#include "MyMenuGameMode.h"
#include "MyMainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void AMyMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!NewPlayer)
		return;

	bool bIsPIE = GetWorld() && GetWorld()->IsPlayInEditor();
	if (!bIsPIE && !NewPlayer->IsLocalPlayerController())
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

	bool bIsPIE = GetWorld() && GetWorld()->IsPlayInEditor();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
			continue;
		if (!bIsPIE && !PC->IsLocalPlayerController())
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
