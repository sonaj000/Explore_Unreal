// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Public/MyCharacterMovementComponent.h"
#include "Engine/DataTable.h"
#include "TestGroundGameMode.generated.h"

class FSQLiteDatabase;
class UMySaveGame;
class ATestGroundCharacter;
class UDataTable;

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
	//we can save the game state to a database. wowwwww

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Analysis")
	UDataTable* PlayerTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Analysis")
	UDataTable* CountTable;

public:
	UFUNCTION()
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY()
	ATestGroundCharacter* MCharacter;

	UFUNCTION(Exec)
	void ExportData();

	UFUNCTION(Exec)
	void ImportData();

	UFUNCTION(Exec)
	void Draw();

	UFUNCTION(Exec)
	void Clear();

	virtual void StartPlay() override;

	UPROPERTY(EditAnywhere, Category = "Character")
	TSubclassOf<ACharacter>CharacterClass; //the character selection.

	UFUNCTION(Exec)
	void P();

private:

	UFUNCTION()
	FString CreateUniqueFolder();

	FString SessionName;

	int counter = 0;

};



