#pragma once

#include "CoreMinimal.h"
#include "MyCharacter.h"
#include "MyNetworkedCharacter.generated.h"

class AShooterWeapon;

UCLASS()
class FPS_API AMyNetworkedCharacter : public AMyCharacter
{
	GENERATED_BODY()

public:
	AMyNetworkedCharacter();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UFUNCTION(Server, Reliable)
	void Server_StartFiring();

	UFUNCTION(Server, Reliable)
	void Server_StopFiring();

	void Input_StartFiring();
	void Input_StopFiring();
	void Input_SwitchWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedWeapon)
	TObjectPtr<AShooterWeapon> ReplicatedWeapon;

	UFUNCTION()
	void OnRep_ReplicatedWeapon();

	void ApplyReplicatedWeapon();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
