#include "MyPlayerState.h"
#include "MyCharacter.h"
#include "MyGameStateBase.h"
#include "Net/UnrealNetwork.h"

AMyPlayerState::AMyPlayerState()
{
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, WeaponTier);
}

void AMyPlayerState::AddScore(int32 Amount)
{
	if (HasAuthority())
	{
		float NewScore = GetScore() + Amount;
		SetScore(NewScore);
		WeaponTier = GetTierForScore(FMath::RoundToInt(NewScore));
		OnRep_WeaponTier();

		if (NewScore >= 10.0f)
		{
			if (AMyGameStateBase* GS = GetWorld()->GetGameState<AMyGameStateBase>())
			{
				GS->OnPlayerVictory(this);
			}
		}
	}
}

void AMyPlayerState::ResetScore()
{
	if (HasAuthority())
	{
		SetScore(0);
		WeaponTier = 0;
		OnRep_WeaponTier();
	}
}

void AMyPlayerState::OnRep_WeaponTier()
{
	if (AMyCharacter* Character = Cast<AMyCharacter>(GetPawn()))
	{
		Character->UpdateBulletTier(WeaponTier);
	}
}

int32 AMyPlayerState::GetTierForScore(int32 InScore) const
{
	if (InScore >= 9) return 3;
	if (InScore >= 6) return 2;
	if (InScore >= 3) return 1;
	return 0;
}
