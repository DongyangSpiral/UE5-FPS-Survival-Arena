#include "MyGameModeBase.h"
#include "MyCharacter.h"
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

	if (AMyGameStateBase* GS = GetGameState<AMyGameStateBase>())
		GS->ResetState();
}

void AMyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void AMyGameModeBase::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (HasAuthority())
	{
		if (APawn* Pawn = NewPlayer->GetPawn())
		{
			if (AMyCharacter* Char = Cast<AMyCharacter>(Pawn))
				Char->OnCharacterDeath.AddUObject(this, &AMyGameModeBase::HandleCharacterDeath);
		}

		if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
		{
			AlivePlayers.Add(PC);
			SyncAliveCount();
		}
	}
}

void AMyGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (HasAuthority())
	{
		AlivePlayers.Remove(Cast<APlayerController>(Exiting));
		SyncAliveCount();
	}
}

void AMyGameModeBase::HandleCharacterDeath(AController* DeadPlayer)
{
	if (!HasAuthority() || !DeadPlayer)
		return;

	AlivePlayers.Remove(Cast<APlayerController>(DeadPlayer));
	SyncAliveCount();
}

void AMyGameModeBase::SyncAliveCount()
{
	if (AMyGameStateBase* GS = GetGameState<AMyGameStateBase>())
		GS->SetAlivePlayerCount(AlivePlayers.Num());
}
