#include "MyGameModeBase.h"
#include "MyGameStateBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AMyGameModeBase::AMyGameModeBase()
{
	GameStateClass = AMyGameStateBase::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PawnClass(
		TEXT("/Game/BP_SurvivalCharacter.BP_SurvivalCharacter_C"));
	if (PawnClass.Succeeded())
		DefaultPawnClass = PawnClass.Class;

	static ConstructorHelpers::FClassFinder<APlayerController> PCClass(
		TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController.BP_FirstPersonPlayerController_C"));
	if (PCClass.Succeeded())
		PlayerControllerClass = PCClass.Class;
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	MyGameState = GetGameState<AMyGameStateBase>();
	InitAlivePlayerCount();
}

void AMyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (MyGameState)
	{
		MyGameState->OnPlayerRespawned();
	}
}

void AMyGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (MyGameState && Exiting && Exiting->PlayerState)
	{
		MyGameState->OnPlayerDied();
	}
}

void AMyGameModeBase::InitAlivePlayerCount()
{
	if (!MyGameState)
		return;

	int32 Count = 0;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (PS && !PS->IsInactive())
			Count++;
	}
	MyGameState->AlivePlayerCount = Count;
}
