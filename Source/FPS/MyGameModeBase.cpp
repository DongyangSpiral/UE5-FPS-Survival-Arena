#include "MyGameModeBase.h"
#include "MyPlayerState.h"
#include "MyGameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AMyGameModeBase::AMyGameModeBase()
{
	GameStateClass = AMyGameStateBase::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PawnClass(
		TEXT("/Game/BP_SurvivalCharacter.BP_SurvivalCharacter_C"));
	if (PawnClass.Succeeded())
		DefaultPawnClass = PawnClass.Class;

	static ConstructorHelpers::FClassFinder<APlayerController> PCClass(
		TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController.BP_FirstPersonPlayerController_C"));
	if (PCClass.Succeeded())
		PlayerControllerClass = PCClass.Class;
}
