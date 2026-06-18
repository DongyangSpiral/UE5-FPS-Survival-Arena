#include "MyLeaderboard.h"
#include "MyUIWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateColor.h"

void UMyLeaderboard::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Root canvas — takes full viewport, positions children via anchors
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	// Outer border (semi-transparent background)
	UBorder* Bg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Bg"));
	Bg->SetContentColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.55f));

	// Main vertical layout
	UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainBox"));

	// Title row
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	Title->SetText(FText::FromString(TEXT("Leaderboard")));
	Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 20));
	Title->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
	{
		UVerticalBoxSlot* TitleSlot = MainBox->AddChildToVerticalBox(Title);
		TitleSlot->SetPadding(FMargin(8.f, 6.f, 8.f, 2.f));
	}

	// Target score text
	TargetScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TargetScoreText"));
	TargetScoreText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 13));
	TargetScoreText->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f));
	{
		UVerticalBoxSlot* TargetSlot = MainBox->AddChildToVerticalBox(TargetScoreText);
		TargetSlot->SetPadding(FMargin(8.f, 0.f, 8.f, 4.f));
	}

	// Separator line
	UBorder* Separator = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Separator"));
	Separator->SetContentColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.f));
	{
		UVerticalBoxSlot* SepSlot = MainBox->AddChildToVerticalBox(Separator);
		SepSlot->SetPadding(FMargin(4.f, 0.f, 4.f, 0.f));
		FSlateChildSize SepSize(ESlateSizeRule::Fill);
		SepSize.Value = 0.01f;
		SepSlot->SetSize(SepSize);
	}

	// Entry list
	EntryList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EntryList"));
	{
		UVerticalBoxSlot* ListSlot = MainBox->AddChildToVerticalBox(EntryList);
		ListSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 4.f));
	}

	Bg->SetContent(MainBox);

	// Anchor top-right, 10px inset
	UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
	BgSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
	BgSlot->SetAlignment(FVector2D(1.f, 0.f));
	BgSlot->SetPosition(FVector2D(-10.f, 60.f));
	BgSlot->SetAutoSize(true);
}

void UMyLeaderboard::UpdateEntries(const TArray<FLeaderboardEntry>& Entries, int32 TargetScore)
{
	if (!EntryList)
		return;

	if (TargetScoreText)
	{
		TargetScoreText->SetText(FText::Format(
			FText::FromString(TEXT("Target: {0} pts")),
			FText::AsNumber(TargetScore)
		));
	}

	EntryList->ClearChildren();

	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		const FLeaderboardEntry& Entry = Entries[i];

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

		// Rank number
		UTextBlock* RankText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		RankText->SetText(FText::AsNumber(i + 1));
		RankText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
		{
			UHorizontalBoxSlot* RankSlot = Row->AddChildToHorizontalBox(RankText);
			RankSlot->SetPadding(FMargin(8.f, 2.f, 4.f, 2.f));
			FSlateChildSize RankSize(ESlateSizeRule::Fill);
			RankSize.Value = 0.15f;
			RankSlot->SetSize(RankSize);
		}

		// Player name
		UTextBlock* NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		NameText->SetText(FText::FromString(Entry.PlayerName));
		NameText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
		{
			UHorizontalBoxSlot* NameSlot = Row->AddChildToHorizontalBox(NameText);
			NameSlot->SetPadding(FMargin(4.f, 2.f, 4.f, 2.f));
			FSlateChildSize NameSize(ESlateSizeRule::Fill);
			NameSize.Value = 0.55f;
			NameSlot->SetSize(NameSize);
		}

		// Score
		UTextBlock* ScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		ScoreText->SetText(FText::AsNumber(Entry.Score));
		ScoreText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 14));
		{
			UHorizontalBoxSlot* ScoreSlot = Row->AddChildToHorizontalBox(ScoreText);
			ScoreSlot->SetPadding(FMargin(4.f, 2.f, 8.f, 2.f));
			FSlateChildSize ScoreSize(ESlateSizeRule::Fill);
			ScoreSize.Value = 0.3f;
			ScoreSlot->SetSize(ScoreSize);
			ScoreSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		}

		// Highlight local player
		if (Entry.bIsLocalPlayer)
		{
			NameText->SetColorAndOpacity(FLinearColor(1.f, 0.9f, 0.f));
			RankText->SetColorAndOpacity(FLinearColor(1.f, 0.9f, 0.f));
			ScoreText->SetColorAndOpacity(FLinearColor(1.f, 0.9f, 0.f));
		}

		EntryList->AddChildToVerticalBox(Row);
	}
}
