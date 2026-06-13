#include "MyNetworkedCharacter.h"
#include "FPS.h"
#include "MyGameStateBase.h"
#include "MyPlayerState.h"
#include "MyUIWidget.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"

AMyNetworkedCharacter::AMyNetworkedCharacter()
{
}

void AMyNetworkedCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->SetReplicates(true);
			ReplicatedWeapon = CurrentWeapon;
		}
	}
}

void AMyNetworkedCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	AFPSCharacter::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &AMyNetworkedCharacter::Input_StartFiring);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &AMyNetworkedCharacter::Input_StopFiring);
		EnhancedInput->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AMyNetworkedCharacter::Input_SwitchWeapon);
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (DefaultIMC) { Subsystem->RemoveMappingContext(DefaultIMC); Subsystem->AddMappingContext(DefaultIMC, 0); }
		if (WeaponIMC)  { Subsystem->RemoveMappingContext(WeaponIMC);  Subsystem->AddMappingContext(WeaponIMC, 0); }
	}

	if (IsLocallyControlled() && MyWidgetClass)
	{
		if (AMyPlayerState* PS = GetPlayerState<AMyPlayerState>())
		{
			if (!PS->MyUIWidget)
			{
				PS->MyUIWidget = CreateWidget<UMyUIWidget>(PC, MyWidgetClass);
				if (PS->MyUIWidget)
					PS->MyUIWidget->AddToPlayerScreen(0);
			}
		}
	}

}

void AMyNetworkedCharacter::Input_StartFiring()
{
	if (!CurrentWeapon || IsDead())
		return;

	CurrentWeapon->StartFiring();

	if (!HasAuthority())
		Server_StartFiring();
}

void AMyNetworkedCharacter::Input_StopFiring()
{
	if (!CurrentWeapon || IsDead())
		return;

	CurrentWeapon->StopFiring();

	if (!HasAuthority())
		Server_StopFiring();
}

void AMyNetworkedCharacter::Input_SwitchWeapon()
{
	if (OwnedWeapons.Num() > 1 && !IsDead())
	{
		CurrentWeapon->DeactivateWeapon();
		int32 Index = OwnedWeapons.Find(CurrentWeapon);
		Index = (Index == OwnedWeapons.Num() - 1) ? 0 : Index + 1;
		CurrentWeapon = OwnedWeapons[Index];
		CurrentWeapon->ActivateWeapon();

		if (HasAuthority())
			ReplicatedWeapon = CurrentWeapon;
	}
}

void AMyNetworkedCharacter::Server_StartFiring_Implementation()
{
	if (CurrentWeapon && !IsDead())
		CurrentWeapon->StartFiring();
}

void AMyNetworkedCharacter::Server_StopFiring_Implementation()
{
	if (CurrentWeapon && !IsDead())
		CurrentWeapon->StopFiring();
}

void AMyNetworkedCharacter::OnRep_ReplicatedWeapon()
{
	if (!ReplicatedWeapon || HasAuthority())
		return;

	if (!OwnedWeapons.Contains(ReplicatedWeapon))
		OwnedWeapons.Add(ReplicatedWeapon);

	if (CurrentWeapon != ReplicatedWeapon)
	{
		CurrentWeapon = ReplicatedWeapon;
	}
}

void AMyNetworkedCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyNetworkedCharacter, ReplicatedWeapon);
}
