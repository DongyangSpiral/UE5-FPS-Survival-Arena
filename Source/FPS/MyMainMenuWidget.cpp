#include "MyMainMenuWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UMyMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = RootCanvas;

	// Background
	UBorder* Bg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
	Bg->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.1f, 1.f));
	{
		UCanvasPanelSlot* CS = RootCanvas->AddChildToCanvas(Bg);
		CS->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CS->SetOffsets(FMargin(0.f));
	}

	// Center content
	UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainBox"));
	{
		UCanvasPanelSlot* CS = RootCanvas->AddChildToCanvas(MainBox);
		CS->SetAnchors(FAnchors(0.5f, 0.45f, 0.5f, 0.45f));
		CS->SetAlignment(FVector2D(0.5f, 0.5f));
		CS->SetPosition(FVector2D(0.f, 0.f));
		CS->SetAutoSize(true);
	}

	// Title
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	Title->SetText(FText::FromString(TEXT("SURVIVAL ARENA")));
	Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 36));
	Title->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.95f));
	Title->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(Title);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 40.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Create button
	UButton* CreateBtn = nullptr;
	{
		UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Txt->SetText(FText::FromString(TEXT("Create Room")));
		Txt->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
		Txt->SetJustification(ETextJustify::Center);
		Txt->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
		Btn->SetContent(Txt);

		FButtonStyle Style;
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.3f, 1.f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.35f, 0.35f, 0.5f, 1.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.f));
		Btn->WidgetStyle = Style;
		Btn->OnClicked.AddDynamic(this, &UMyMainMenuWidget::OnCreateRoom);

		USizeBox* SB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SB->SetWidthOverride(260.f);
		SB->SetHeightOverride(50.f);
		SB->SetContent(Btn);
		CreateBtn = Btn;

		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(SB);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Join button
	UButton* JoinBtn = nullptr;
	{
		UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Txt->SetText(FText::FromString(TEXT("Join Room")));
		Txt->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
		Txt->SetJustification(ETextJustify::Center);
		Txt->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
		Btn->SetContent(Txt);

		FButtonStyle Style;
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.3f, 1.f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.35f, 0.35f, 0.5f, 1.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.f));
		Btn->WidgetStyle = Style;
		Btn->OnClicked.AddDynamic(this, &UMyMainMenuWidget::OnJoinRoom);

		USizeBox* SB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SB->SetWidthOverride(260.f);
		SB->SetHeightOverride(50.f);
		SB->SetContent(Btn);

		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(SB);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Quit button
	{
		UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Txt->SetText(FText::FromString(TEXT("Quit")));
		Txt->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
		Txt->SetJustification(ETextJustify::Center);
		Txt->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
		Btn->SetContent(Txt);

		FButtonStyle Style;
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.3f, 1.f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.35f, 0.35f, 0.5f, 1.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.f));
		Btn->WidgetStyle = Style;
		Btn->OnClicked.AddDynamic(this, &UMyMainMenuWidget::OnQuit);

		USizeBox* SB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SB->SetWidthOverride(260.f);
		SB->SetHeightOverride(50.f);
		SB->SetContent(Btn);

		UVerticalBoxSlot* VS = MainBox->AddChildToVerticalBox(SB);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 0.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Join overlay (initially hidden)
	OverlayContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("OverlayContainer"));
	OverlayContainer->SetVisibility(ESlateVisibility::Collapsed);
	{
		UCanvasPanelSlot* CS = RootCanvas->AddChildToCanvas(OverlayContainer);
		CS->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CS->SetAlignment(FVector2D(0.f, 0.f));
		CS->SetPosition(FVector2D(0.f, 0.f));
		CS->SetAutoSize(false);
	}

	// Overlay background
	UBorder* OverlayBg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OverlayBg"));
	OverlayBg->SetContentColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.6f));
	{
		UCanvasPanelSlot* CS = OverlayContainer->AddChildToCanvas(OverlayBg);
		CS->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CS->SetAlignment(FVector2D(0.f, 0.f));
		CS->SetPosition(FVector2D(0.f, 0.f));
		CS->SetAutoSize(false);
	}

	// Overlay content
	UVerticalBox* OverlayBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OverlayBox"));
	{
		UCanvasPanelSlot* CS = OverlayContainer->AddChildToCanvas(OverlayBox);
		CS->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		CS->SetAlignment(FVector2D(0.5f, 0.5f));
		CS->SetPosition(FVector2D(0.f, 0.f));
		CS->SetAutoSize(true);
	}

	// Overlay title
	UTextBlock* OverlayTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OverlayTitle"));
	OverlayTitle->SetText(FText::FromString(TEXT("Enter Server IP")));
	OverlayTitle->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 22));
	OverlayTitle->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
	OverlayTitle->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VS = OverlayBox->AddChildToVerticalBox(OverlayTitle);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// IP input
	IPInputBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("IPInput"));
	IPInputBox->SetHintText(FText::FromString(TEXT("192.168.")));

	USizeBox* InputSB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	InputSB->SetWidthOverride(300.f);
	InputSB->SetContent(IPInputBox);
	{
		UVerticalBoxSlot* VS = OverlayBox->AddChildToVerticalBox(InputSB);
		VS->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Overlay buttons row
	UHorizontalBox* OverlayBtnRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("OverlayBtnRow"));
	{
		UVerticalBoxSlot* VS = OverlayBox->AddChildToVerticalBox(OverlayBtnRow);
		VS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	// Connect button
	{
		UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Txt->SetText(FText::FromString(TEXT("Connect")));
		Txt->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
		Txt->SetJustification(ETextJustify::Center);
		Txt->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
		Btn->SetContent(Txt);

		FButtonStyle Style;
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.3f, 1.f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.35f, 0.35f, 0.5f, 1.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.f));
		Btn->WidgetStyle = Style;
		Btn->OnClicked.AddDynamic(this, &UMyMainMenuWidget::OnConnect);

		USizeBox* SB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SB->SetWidthOverride(130.f);
		SB->SetHeightOverride(50.f);
		SB->SetContent(Btn);

		UHorizontalBoxSlot* HS = OverlayBtnRow->AddChildToHorizontalBox(SB);
		HS->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
	}

	// Cancel button
	{
		UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Txt->SetText(FText::FromString(TEXT("Cancel")));
		Txt->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
		Txt->SetJustification(ETextJustify::Center);
		Txt->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
		Btn->SetContent(Txt);

		FButtonStyle Style;
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.3f, 1.f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.35f, 0.35f, 0.5f, 1.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.f));
		Btn->WidgetStyle = Style;
		Btn->OnClicked.AddDynamic(this, &UMyMainMenuWidget::OnCancelJoin);

		USizeBox* SB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SB->SetWidthOverride(130.f);
		SB->SetHeightOverride(50.f);
		SB->SetContent(Btn);

		UHorizontalBoxSlot* HS = OverlayBtnRow->AddChildToHorizontalBox(SB);
		HS->SetPadding(FMargin(10.f, 0.f, 0.f, 0.f));
	}
}

void UMyMainMenuWidget::OnCreateRoom()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
	GetWorld()->ServerTravel(TEXT("/Game/Lvl_Lobby?listen"));
}

void UMyMainMenuWidget::OnJoinRoom()
{
	ShowJoinOverlay();
}

void UMyMainMenuWidget::OnQuit()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->ConsoleCommand(TEXT("quit"));
	}
}

void UMyMainMenuWidget::OnConnect()
{
	if (!IPInputBox) return;

	FString IP = IPInputBox->GetText().ToString();
	IP.TrimStartAndEndInline();
	if (IP.IsEmpty())
		return;

	if (!IP.Contains(TEXT(".")))
		return;

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		PC->ClientTravel(IP, ETravelType::TRAVEL_Absolute);
	}
}

void UMyMainMenuWidget::OnCancelJoin()
{
	HideJoinOverlay();
}

void UMyMainMenuWidget::ShowJoinOverlay()
{
	if (OverlayContainer)
	{
		OverlayContainer->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMyMainMenuWidget::HideJoinOverlay()
{
	if (OverlayContainer)
	{
		OverlayContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
}
