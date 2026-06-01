#include "MyPlayerController.h"
#include "MyUIWidget.h"
#include "MyCharacter.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "GameFramework/Pawn.h"

AMyPlayerController::AMyPlayerController()
{
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController() && MyUIClass)
	{
		MyUI = CreateWidget<UMyUIWidget>(this, MyUIClass);
		if (MyUI)
		{
			MyUI->AddToPlayerScreen(0);
		}
	}
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	InPawn->OnDestroyed.AddDynamic(this, &AMyPlayerController::OnPawnDestroyed);

	if (AShooterCharacter* Char = Cast<AShooterCharacter>(InPawn))
	{
		Char->OnBulletCountUpdated.AddDynamic(this, &AMyPlayerController::OnBulletCountUpdated);
		Char->OnDamaged.AddDynamic(this, &AMyPlayerController::OnPawnDamaged);

		if (AMyCharacter* MyChar = Cast<AMyCharacter>(InPawn))
		{
			MyChar->OnMyCharacterDied.AddDynamic(this, &AMyPlayerController::OnMyCharacterDied);
			MyChar->OnBulletTierChanged.AddDynamic(this, &AMyPlayerController::OnBulletTierChanged);

			if (MyUI)
			{
				MyUI->UpdateHealth(MyChar->GetLifePercent());

				if (AShooterWeapon* Weapon = MyChar->GetCurrentWeapon())
				{
					OnBulletCountUpdated(Weapon->GetMagazineSize(), Weapon->GetBulletCount());
				}
			}
		}
	}
}

void AMyPlayerController::OnUnPossess()
{
	if (MyUI)
	{
		MyUI->BP_HideDeathOverlay();
	}
	Super::OnUnPossess();
}

void AMyPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	if (MyUI)
	{
		MyUI->BP_HideDeathOverlay();
	}
}

void AMyPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	if (MyUI)
	{
		MyUI->UpdateBulletCount(Bullets, MagazineSize);
	}
}

void AMyPlayerController::OnPawnDamaged(float LifePercent)
{
	if (MyUI)
	{
		MyUI->UpdateHealth(LifePercent);
	}
}

void AMyPlayerController::OnMyCharacterDied(float RespawnTime)
{
	if (MyUI)
	{
		MyUI->ShowDeathOverlay(RespawnTime);
	}
}

void AMyPlayerController::OnBulletTierChanged(int32 NewTier)
{
	if (MyUI)
	{
		MyUI->UpdateBulletTier(NewTier);
		MyUI->BP_ShowTierUp(NewTier);
	}
}
