#include "MyCharacter.h"
#include "FPS.h"
#include "MyPlayerState.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Variant_Shooter/Weapons/ShooterProjectile.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
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
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
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
		{
			PS->ResetScore();
		}

		Tags.Add(DeathTag);
		GetCharacterMovement()->StopMovementImmediately();
		DisableInput(Cast<APlayerController>(GetController()));
		BP_OnDeath();

		OnMyCharacterDied.Broadcast(RespawnTime);

		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AMyCharacter::OnRespawn, RespawnTime, false);
	}

	return Damage;
}

void AMyCharacter::OnRespawn()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		PC->UnPossess();
		Destroy();
		if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
			GM->RestartPlayer(PC);
	}
}
