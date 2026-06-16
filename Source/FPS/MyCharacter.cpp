#include "MyCharacter.h"
#include "FPS.h"
#include "MyPlayerState.h"
#include "MyUIWidget.h"
#include "MyGameStateBase.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Variant_Shooter/Weapons/ShooterProjectile.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Net/UnrealNetwork.h"
#include "FPSCharacter.h"

AMyCharacter::AMyCharacter()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> DefaultArms(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (DefaultArms.Succeeded())
	{
		GetFirstPersonMesh()->SetSkeletalMesh(DefaultArms.Object);
	}

	GetFirstPersonMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FClassFinder<UAnimInstance> DefaultAnim(
		TEXT("/Game/FirstPerson/Anims/ABP_FP_Copy.ABP_FP_Copy_C"));
	if (DefaultAnim.Succeeded())
	{
		GetFirstPersonMesh()->AnimClass = DefaultAnim.Class;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Jump(
		TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	if (IA_Jump.Succeeded())
		JumpAction = IA_Jump.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Move(
		TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	if (IA_Move.Succeeded())
		MoveAction = IA_Move.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Look(
		TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	if (IA_Look.Succeeded())
		LookAction = IA_Look.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_MouseLook(
		TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	if (IA_MouseLook.Succeeded())
		MouseLookAction = IA_MouseLook.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Shoot(
		TEXT("/Game/Variant_Shooter/Input/Actions/IA_Shoot.IA_Shoot"));
	if (IA_Shoot.Succeeded())
		FireAction = IA_Shoot.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Swap(
		TEXT("/Game/Variant_Shooter/Input/Actions/IA_SwapWeapon.IA_SwapWeapon"));
	if (IA_Swap.Succeeded())
		SwitchWeaponAction = IA_Swap.Object;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_Default(
		TEXT("/Game/Input/IMC_Default.IMC_Default"));
	if (IMC_Default.Succeeded())
		DefaultIMC = IMC_Default.Object;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_Weapons(
		TEXT("/Game/Variant_Shooter/Input/IMC_Weapons.IMC_Weapons"));
	if (IMC_Weapons.Succeeded())
		WeaponIMC = IMC_Weapons.Object;

	static ConstructorHelpers::FClassFinder<AShooterWeapon> FallbackWeapon(
		TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Weapons/BP_ShooterWeapon_Rifle.BP_ShooterWeapon_Rifle_C"));
	if (FallbackWeapon.Succeeded())
		FallbackWeaponClass = FallbackWeapon.Class;

}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("Player"));

	if (HasAuthority())
	{
		GiveWeapon();
		if (CurrentWeapon)
			ReplicatedWeapon = CurrentWeapon;
	}
	else
	{
		ApplyReplicatedWeapon();
	}
}

void AMyCharacter::GiveWeapon()
{
	TSubclassOf<AShooterWeapon> WeaponToGive = DefaultWeaponClass;
	if (!WeaponToGive)
		WeaponToGive = FallbackWeaponClass;

	if (WeaponToGive)
	{
		AddWeaponClass(WeaponToGive);
	}

	if (AMyPlayerState* PS = GetPlayerState<AMyPlayerState>())
	{
		UpdateBulletTier(PS->WeaponTier);
	}
}

void AMyCharacter::UpdateBulletTier(int32 NewTier)
{
	if (!CurrentWeapon || NewTier < 0 || NewTier >= 4)
		return;

	if (!TierProjectileClasses[NewTier])
		return;

	FObjectProperty* Prop = FindFProperty<FObjectProperty>(AShooterWeapon::StaticClass(), TEXT("ProjectileClass"));
	if (Prop)
	{
		void* Addr = Prop->ContainerPtrToValuePtr<UObject*>(CurrentWeapon);
		Prop->SetObjectPropertyValue(Addr, TierProjectileClasses[NewTier].Get());
		UE_LOG(LogFPS, Verbose, TEXT("AMyCharacter::UpdateBulletTier - Set ProjectileClass to tier %d"), NewTier);
	}

	OnBulletTierChanged.Broadcast(NewTier);
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	AFPSCharacter::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &AMyCharacter::Input_StartFiring);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &AMyCharacter::Input_StopFiring);
		EnhancedInput->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AMyCharacter::Input_SwitchWeapon);
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (DefaultIMC)
		{
			Subsystem->RemoveMappingContext(DefaultIMC);
			Subsystem->AddMappingContext(DefaultIMC, 0);
		}
		if (WeaponIMC)
		{
			Subsystem->RemoveMappingContext(WeaponIMC);
			Subsystem->AddMappingContext(WeaponIMC, 0);
		}
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

void AMyCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	if (Weapon->GetFirstPersonAnimInstanceClass())
		GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	if (Weapon->GetThirdPersonAnimInstanceClass())
		GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

float AMyCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CurrentHP <= 0.0f)
		return 0.0f;

	CurrentHP -= Damage;
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	if (CurrentHP <= 0.0f)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->DeactivateWeapon();
			CurrentWeapon->GetFirstPersonMesh()->SetAnimInstanceClass(nullptr);
			CurrentWeapon->GetThirdPersonMesh()->SetAnimInstanceClass(nullptr);
			CurrentWeapon->Destroy();
		}

		OwnedWeapons.Empty();
		CurrentWeapon = nullptr;

		GetFirstPersonMesh()->SetAnimInstanceClass(nullptr);
		GetMesh()->SetAnimInstanceClass(nullptr);

		if (AMyPlayerState* PS = GetPlayerState<AMyPlayerState>())
			PS->ResetScore();

		Tags.Add(DeathTag);
		GetCharacterMovement()->StopMovementImmediately();
		DisableInput(Cast<APlayerController>(GetController()));
		BP_OnDeath();

		OnMyCharacterDied.Broadcast(RespawnTime);

		if (HasAuthority())
			OnCharacterDeath.Broadcast(GetController());

		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AMyCharacter::OnRespawn, RespawnTime, false);
	}

	return Damage;
}

void AMyCharacter::OnRespawn()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (AMyGameStateBase* GS = GetWorld()->GetGameState<AMyGameStateBase>())
		{
			if (GS->bGameFinished)
				return;
		}
		Destroy();
		if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
			GM->RestartPlayer(PC);
	}
}

void AMyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AMyCharacter::Input_StartFiring()
{
	if (!CurrentWeapon || IsDead())
		return;

	CurrentWeapon->StartFiring();

	if (!HasAuthority())
		Server_StartFiring();
}

void AMyCharacter::Input_StopFiring()
{
	if (!CurrentWeapon || IsDead())
		return;

	CurrentWeapon->StopFiring();

	if (!HasAuthority())
		Server_StopFiring();
}

void AMyCharacter::Input_SwitchWeapon()
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

		OnWeaponActivated(CurrentWeapon);
	}
}

void AMyCharacter::Server_StartFiring_Implementation()
{
	if (CurrentWeapon && !IsDead())
		CurrentWeapon->StartFiring();
}

void AMyCharacter::Server_StopFiring_Implementation()
{
	if (CurrentWeapon && !IsDead())
		CurrentWeapon->StopFiring();
}

void AMyCharacter::OnRep_ReplicatedWeapon()
{
	ApplyReplicatedWeapon();
}

void AMyCharacter::ApplyReplicatedWeapon()
{
	if (HasAuthority() || !ReplicatedWeapon)
		return;

	if (!OwnedWeapons.Contains(ReplicatedWeapon))
		OwnedWeapons.Add(ReplicatedWeapon);

	if (CurrentWeapon != ReplicatedWeapon)
	{
		if (CurrentWeapon)
			CurrentWeapon->DeactivateWeapon();
		CurrentWeapon = ReplicatedWeapon;
	}

	OnWeaponActivated(CurrentWeapon);

	if (AMyPlayerState* PS = GetPlayerState<AMyPlayerState>())
		UpdateBulletTier(PS->WeaponTier);
}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyCharacter, ReplicatedWeapon);
}
