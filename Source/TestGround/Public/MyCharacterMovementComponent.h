// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCharacterMovementComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Hang			UMETA(DisplayName = "Hang"),
	CMOVE_WallRun		UMETA(DisplayName = "WallRun"),

};

UCLASS()
class TESTGROUND_API UMyCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()


public:
	UMyCharacterMovementComponent();

	UPROPERTY(Transient)
	class ATestGroundCharacter* TestGroundOwner;

	UPROPERTY(EditDefaultsOnly) 
	float MaxSprintSpeed = 750.f;\

	UPROPERTY()
	bool Safe_bWallRunRight;

	UPROPERTY()
	bool bWantsToSprint;
public:

	bool btest = true;

	UPROPERTY(EditDefaultsOnly)
	float Sprint_MaxWalkSpeed;

	UPROPERTY(EditDefaultsOnly)
	float Walk_MaxWalkSpeed;

	UPROPERTY(EditDefaultsOnly)
	float MinWallRunSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly)
	float MaxWallRunSpeed = 800.0f;

	UPROPERTY(EditDefaultsOnly)
	float MaxVerticalWallRunSpeed = 200.0f; 

	UPROPERTY(EditDefaultsOnly)
	float WallRunPullAwayAngle = 75.0f; //the angle in which u leave the wall

	UPROPERTY(EditDefaultsOnly)
	float WallAttractionForce = 200.0f; //a force that pull you to the wall for curved walls

	UPROPERTY(EditDefaultsOnly)
	float MinWallRunHeight = 50.0f; 

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* WallRunGravityScaleCurve; //vertical gravity to apply

	UPROPERTY(EditDefaultsOnly)
	float WallJumpOffForce = 300.0f;
public:

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);

	virtual float GetMaxSpeed() const override;

	UFUNCTION(BlueprintPure)
	bool IsMovementMode(EMovementMode inMovementMode) const;

	UFUNCTION(BlueprintPure) 
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

	UFUNCTION(BlueprintPure)
	uint8 GetCustomMovementMode() const;

public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

public:

	UFUNCTION(BlueprintCallable)
	void SprintPressed();

	UFUNCTION(BlueprintCallable)
	void SprintReleased();

	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

	virtual float GetMaxBrakingDeceleration() const override;

	virtual bool CanAttemptJump() const override;

	virtual bool DoJump(bool bReplayingMoves) override;

protected:
//Wall Run
	bool TryWallRun();

	void PhysWallRun(float deltaTime, int32 Iterations);

	UFUNCTION(BlueprintPure)
	bool isWallRunning() const { return IsCustomMovementMode(CMOVE_WallRun); }

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

private:
	float CapR() const;
	float CapHH() const;



	
};
