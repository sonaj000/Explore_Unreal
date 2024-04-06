// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Engine/DataTable.h"

#include "TestGroundCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

class UMyCharacterMovementComponent;
class ATestGroundGameMode;
class UMySaveGame;
class APressurePlate;

USTRUCT(BlueprintType)
struct FPlayerStateTable : public FTableRowBase
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FString CellString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int VX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int VY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int VZ;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int pitch;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int yaw;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int roll;
};


UCLASS(config=Game)
class ATestGroundCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TestAction;

public:
	ATestGroundCharacter(const FObjectInitializer& ObjectInitializer);

	FCollisionQueryParams GetIgnoreCharacterParams() const;
	
	UFUNCTION(BlueprintPure) //only callable in bp
	FORCEINLINE UMyCharacterMovementComponent* GetTGCCharacterMovement() const { return TGCMovementComponent; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	UMyCharacterMovementComponent* TGCMovementComponent;

	UPROPERTY()
	bool bcanJump;

	TArray<int>StatHolder;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	virtual void Jump() override;

	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION()
	void Sprint();

	UFUNCTION()
	void StopSprinting();

	UFUNCTION()
	void TestFunction();

	UFUNCTION()
	void SpawnDebugBoxForCell(FVector cell);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) //counter for FNAME purposes for adding row name
		int nameCounter = 0;
private:

	UFUNCTION()
	UMySaveGame* GetStateAsSave();

	UFUNCTION()
	void RestoreStateFromSave(UMySaveGame* Save);

	UPROPERTY()
	ATestGroundGameMode* CurrentGameMode;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	UFUNCTION()
	TArray<FVector> GetCellString();

	UFUNCTION()
	int CellScoreCalculator(FString SelectedCell);

	UPROPERTY()
	APressurePlate* gamebridge;

public:

	TMap<FVector, TArray<UMySaveGame*>>StatesForCells;


	void RememberCurrentState();

	void FastTick();

	void SlowTick();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION()
	virtual void Tick(float DeltaSeconds) override;

protected: //FSM stuff

	UFUNCTION()
	void RandomSeed();

	UFUNCTION()
	void RandomMovement();

	UPROPERTY()
	int RandSeed = 0;

	void NewSeed();
	bool bCanSeed = false;

	UPROPERTY()
	bool bCanGo = false;

	UPROPERTY()
	bool bCanRun = false;

};

