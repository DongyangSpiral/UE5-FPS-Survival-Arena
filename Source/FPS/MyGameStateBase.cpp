#include "MyGameStateBase.h"
#include "FPS.h"
#include "MyPlayerState.h"
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
	DOREPLIFETIME(AMyGameStateBase, AlivePlayerCount);
}

void AMyGameStateBase::ResetState()
{
	if (!HasAuthority())
		return;

	AlivePlayerCount = 0;
	bGameFinished = false;
	WinnerName.Empty();
}

void AMyGameStateBase::SetAlivePlayerCount(int32 NewCount)
{
	if (!HasAuthority())
		return;

	AlivePlayerCount = NewCount;
	OnRep_AliveCount();
	CheckDefeatCondition();
}

void AMyGameStateBase::CheckDefeatCondition()
{
	if (bGameFinished || AlivePlayerCount > 0 || PlayerArray.Num() <= 0)
		return;

	UE_LOG(LogFPS, Warning, TEXT("CheckDefeatCondition triggered - freezing all actors"));

	bGameFinished = true;

	FreezeAllActors();
	Multicast_ShowDefeat();
}

void AMyGameStateBase::OnPlayerVictory(AMyPlayerState* Winner)
{
	if (bGameFinished || !Winner)
		return;

	bGameFinished = true;
	WinnerName = Winner->GetPlayerName();
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

void AMyGameStateBase::OnRep_AliveCount()
{
	OnAliveCountChanged.Broadcast(AlivePlayerCount);
}

void AMyGameStateBase::Multicast_ShowVictory_Implementation(const FString& Name)
{
	OnGameFinished.Broadcast(Name, true);
	BP_OnVictory(Name);
}

void AMyGameStateBase::Multicast_ShowDefeat_Implementation()
{
	OnGameFinished.Broadcast(TEXT(""), false);
	BP_OnDefeat();
}
