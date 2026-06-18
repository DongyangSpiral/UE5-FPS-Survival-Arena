#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

UCLASS()
class FPS_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	FString PendingPlayerName;
};
