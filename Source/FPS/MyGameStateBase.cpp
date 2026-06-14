#include "MyGameStateBase.h"
#include "MyPlayerState.h"
#include "MyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

AMyGameStateBase::AMyGameStateBase()
{
}

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameStateBase, WinnerName);
	DOREPLIFETIME(AMyGameStateBase, bGameFinished);
	DOREPLIFETIME(AMyGameStateBase, TeamScore);
	DOREPLIFETIME(AMyGameStateBase, AlivePlayerCount);
}

void AMyGameStateBase::ResetState()
{
	if (!HasAuthority())
		return;

	TeamScore = 0;
	AlivePlayerCount = 0;
	bGameFinished = false;
	WinnerName.Empty();
	PlayerArray.Empty();
}

void AMyGameStateBase::AddScore(int32 Amount)
{
	if (!HasAuthority())
		return;

	TeamScore += Amount;
	OnRep_TeamScore();

	if (TeamScore >= TargetScore && !bGameFinished)
	{
		bGameFinished = true;

		AMyPlayerState* BestPlayer = nullptr;
		float BestScore = -1.0f;
		for (APlayerState* PS : PlayerArray)
		{
			if (AMyPlayerState* MPS = Cast<AMyPlayerState>(PS))
			{
				if (MPS->GetScore() > BestScore)
				{
					BestScore = MPS->GetScore();
					BestPlayer = MPS;
				}
			}
		}

		WinnerName = BestPlayer ? BestPlayer->GetPlayerName() : TEXT("Team");

		FreezeAllActors();
		Multicast_ShowVictory(WinnerName);
	}
}

void AMyGameStateBase::RecalculateAlivePlayerCount()
{
	if (!HasAuthority())
		return;

	int32 AliveCount = 0;
	bool bAnyPendingRespawn = false;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		if (!PC)
			continue;

		if (const AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>())
		{
			if (PS->bPendingRespawn)
				bAnyPendingRespawn = true;
		}

		const APawn* Pawn = PC->GetPawn();
		if (const AMyCharacter* Char = Cast<AMyCharacter>(Pawn))
		{
			if (!Char->IsDead())
				++AliveCount;
		}
	}

	AlivePlayerCount = AliveCount;
	OnRep_AliveCount();

	if (AliveCount <= 0 && !bAnyPendingRespawn)
	{
		CheckDefeatCondition();
	}
}

void AMyGameStateBase::CheckDefeatCondition()
{
	if (bGameFinished || PlayerArray.Num() <= 0)
		return;

	bGameFinished = true;
	WinnerName.Empty();

	FreezeAllActors();
	Multicast_ShowDefeat();
}

void AMyGameStateBase::OnPlayerDied()
{
	RecalculateAlivePlayerCount();
}

void AMyGameStateBase::OnPlayerRespawned()
{
	RecalculateAlivePlayerCount();
}

void AMyGameStateBase::OnPlayerVictory(AMyPlayerState* Winner)
{
	if (bGameFinished || !Winner)
		return;

	bGameFinished = true;
	WinnerName = Winner->GetPlayerName();
	TeamScore = TargetScore;
	OnRep_TeamScore();
	Multicast_ShowVictory(WinnerName);
}

void AMyGameStateBase::FreezeAllActors()
{
	if (!HasAuthority())
		return;

	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = Cast<APlayerController>(It->Get()))
		{
			PC->SetPause(true);
		}
	}

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);
	for (AActor* Pawn : AllPawns)
	{
		if (APawn* P = Cast<APawn>(Pawn))
		{
			if (P->GetController() && P->GetController()->IsA<AAIController>())
			{
				if (AAIController* AIC = Cast<AAIController>(P->GetController()))
				{
					AIC->StopMovement();
				}
			}
		}
	}
}

void AMyGameStateBase::OnRep_TeamScore()
{
}

void AMyGameStateBase::OnRep_AliveCount()
{
}

void AMyGameStateBase::Multicast_ShowVictory_Implementation(const FString& Name)
{
	BP_OnVictory(Name);
}

void AMyGameStateBase::Multicast_ShowDefeat_Implementation()
{
	BP_OnDefeat();
}
