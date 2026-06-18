#include "MyLobbyWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "SocketSubsystem.h"

void UMyLobbyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	// Background
	UBorder* Bg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Bg"));
	Bg->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.1f, 1.f));
	{
		UCanvasPanelSlot* CS = Root->AddChildToCanvas(Bg);
		CS->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CS->SetOffsets(FMargin(0.f));
	}

	// Center content
	UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainBox"));
	{
		UCanvasPanelSlot* CS = Root->AddChildToCanvas(MainBox);
		CS->SetAnchors(FAnchors(0.5f, 0.45f, 0.5f, 0.45f));
		CS->SetAlignment(FVector2D(0.5f, 0.5f));
		CS->SetAutoSize(true);
	}

	// Title
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	Title->SetText(FText::FromString(TEXT("Game Lobby")));
	Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 28));
	Title->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
	Title->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(Title);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Server IP display
	IPText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("IPText"));
	IPText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
	IPText->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f));
	IPText->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(IPText);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Player list label
	UTextBlock* ListLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ListLabel"));
	ListLabel->SetText(FText::FromString(TEXT("Players")));
	ListLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
	ListLabel->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f));
	{
		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(ListLabel);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Player list container �?wrap in SizeBox for fixed width
	USizeBox* ListSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ListSizeBox"));
	ListSizeBox->SetWidthOverride(350.f);
	{
		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(ListSizeBox);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	UBorder* ListBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ListBorder"));
	ListBorder->SetContentColorAndOpacity(FLinearColor(0.12f, 0.12f, 0.18f, 0.8f));
	ListSizeBox->SetContent(ListBorder);

	EntryList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EntryList"));
	ListBorder->SetContent(EntryList);

	// Start button (host only)
	StartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StartButton"));
	{
		UTextBlock* BtnText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		BtnText->SetText(FText::FromString(TEXT("Start Game")));
		BtnText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 20));
		BtnText->SetJustification(ETextJustify::Center);
		BtnText->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
		StartButton->SetContent(BtnText);

		FButtonStyle Style;
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.15f, 0.4f, 0.15f, 1.f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.2f, 0.55f, 0.2f, 1.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.3f, 0.1f, 1.f));
		StartButton->WidgetStyle = Style;

		StartButton->OnClicked.AddDynamic(this, &UMyLobbyWidget::OnStartGame);
	}

	USizeBox* StartSB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StartBtnSize"));
	StartSB->SetWidthOverride(260.f);
	StartSB->SetHeightOverride(50.f);
	StartSB->SetContent(StartButton);
	{
		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(StartSB);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Waiting text (non-host)
	WaitingText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WaitingText"));
	WaitingText->SetText(FText::FromString(TEXT("Waiting for host to start the game...")));
	WaitingText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
	WaitingText->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f));
	WaitingText->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(WaitingText);
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Initial refresh
	UpdateIPDisplay();
	RefreshPlayerList();
}

void UMyLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ListRefreshTimer -= InDeltaTime;
	if (ListRefreshTimer <= 0.f)
	{
		RefreshPlayerList();
		ListRefreshTimer = 0.3f;
	}
}

void UMyLobbyWidget::OnStartGame()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->HasAuthority())
		return;

	PC->SetInputMode(FInputModeGameOnly());
	PC->SetShowMouseCursor(false);
	GetWorld()->ServerTravel(TEXT("/Game/Game?listen"));
}

void UMyLobbyWidget::RefreshPlayerList()
{
	if (!EntryList)
		return;

	EntryList->ClearChildren();

	UWorld* World = GetWorld();
	if (!World)
		return;

	AGameStateBase* GS = World->GetGameState();
	if (!GS)
		return;

	APlayerController* LocalPC = GetOwningPlayer();

	for (int32 i = 0; i < GS->PlayerArray.Num(); ++i)
	{
		APlayerState* PS = GS->PlayerArray[i];
		if (!PS)
			continue;

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

		UTextBlock* IndexText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		IndexText->SetText(FText::AsNumber(i + 1));
		IndexText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 16));
		{
			UHorizontalBoxSlot* HS = Row->AddChildToHorizontalBox(IndexText);
			HS->SetPadding(FMargin(8.f, 4.f, 4.f, 4.f));
			FSlateChildSize SZ(ESlateSizeRule::Fill);
			SZ.Value = 0.1f;
			HS->SetSize(SZ);
		}

		UTextBlock* NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		FString DisplayName = PS->GetPlayerName();
		if (i == 0)
		{
			DisplayName += TEXT(" (Host)");
		}
		NameText->SetText(FText::FromString(DisplayName));
		NameText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 16));
		{
			UHorizontalBoxSlot* HS = Row->AddChildToHorizontalBox(NameText);
			HS->SetPadding(FMargin(4.f, 4.f, 8.f, 4.f));
			FSlateChildSize SZ(ESlateSizeRule::Fill);
			SZ.Value = 0.9f;
			HS->SetSize(SZ);
		}

		// Highlight local player
		if (LocalPC && PS == LocalPC->PlayerState)
		{
			IndexText->SetColorAndOpacity(FLinearColor(1.f, 0.9f, 0.f));
			NameText->SetColorAndOpacity(FLinearColor(1.f, 0.9f, 0.f));
		}

		EntryList->AddChildToVerticalBox(Row);
	}
}

void UMyLobbyWidget::UpdateIPDisplay()
{
	if (!IPText)
		return;

	ISocketSubsystem* SocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSub)
	{
		IPText->SetText(FText::FromString(TEXT("Server IP: Unknown")));
		return;
	}

	TArray<TSharedPtr<FInternetAddr>> Addresses;
	SocketSub->GetLocalAdapterAddresses(Addresses);

	FString LocalIP;
	for (const auto& Addr : Addresses)
	{
		if (!Addr)
			continue;
		FString IP = Addr->ToString(false);
		if (!IP.StartsWith(TEXT("127.")) && IP != TEXT("0.0.0.0"))
		{
			LocalIP = IP;
			break;
		}
	}

	if (LocalIP.IsEmpty())
	{
		IPText->SetText(FText::FromString(TEXT("Server IP: Unknown")));
	}
	else
	{
		IPText->SetText(FText::Format(
			FText::FromString(TEXT("Server IP: {0}:7777")),
			FText::FromString(LocalIP)
		));
	}
}
