// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MyCharacterMovementComponent.h"
#include "MySaveGame.generated.h"

/**
 * 
 */
UCLASS()
class TESTGROUND_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FName ActorName;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector PlayerLocation;

	UPROPERTY()
	uint8 CurrentMode;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	int BitValue;

};
