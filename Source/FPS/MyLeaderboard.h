#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyLeaderboard.generated.h"

class UVerticalBox;
class UTextBlock;
struct FLeaderboardEntry;

UCLASS()
class FPS_API UMyLeaderboard : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void UpdateEntries(const TArray<FLeaderboardEntry>& Entries, int32 TargetScore);

private:
	UPROPERTY()
	TObjectPtr<UVerticalBox> EntryList;

	UPROPERTY()
	TObjectPtr<UTextBlock> TargetScoreText;
};
