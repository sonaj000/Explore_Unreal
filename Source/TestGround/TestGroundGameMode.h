// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGroundGameMode.generated.h"

class FSQLiteDatabase;
UCLASS(minimalapi)
class ATestGroundGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATestGroundGameMode();
	FSQLiteDatabase* Db;
	/** Initialize the game. This is called before actors' PreInitializeComponents. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void PreLogin(const FString& Options,const FString& Address,const FUniqueNetIdRepl& UniqueId,FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;
};



