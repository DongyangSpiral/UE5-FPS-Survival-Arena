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

	static ConstructorHelpers::FClassFinder<APlayerController> PCClass(
		TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController.BP_FirstPersonPlayerController_C"));
	if (PCClass.Succeeded())
		PlayerControllerClass = PCClass.Class;

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
