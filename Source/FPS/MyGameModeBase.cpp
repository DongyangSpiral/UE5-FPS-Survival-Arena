#include "MyGameModeBase.h"
#include "MyPlayerState.h"
#include "MyGameStateBase.h"
#include "MyPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AMyGameModeBase::AMyGameModeBase()
{
	GameStateClass = AMyGameStateBase::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PawnClass(
		TEXT("/Game/BP_SurvivalCharacter.BP_SurvivalCharacter_C"));
	if (PawnClass.Succeeded())
		DefaultPawnClass = PawnClass.Class;
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AMyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (HasAuthority())
	{
		if (AMyGameStateBase* GS = GetGameState<AMyGameStateBase>())
			GS->OnPlayerRespawned();
	}
}

void AMyGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (HasAuthority())
	{
		if (AMyGameStateBase* GS = GetGameState<AMyGameStateBase>())
			GS->OnPlayerDied();
	}
}
