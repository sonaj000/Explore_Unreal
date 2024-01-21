// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestGroundGameMode.h"
#include "TestGroundCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "SQLiteDatabase.h"
#include "UObject/ConstructorHelpers.h"
#include "MySaveGame.h"

ATestGroundGameMode::ATestGroundGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	Db = nullptr;
	//this two lines help the gamemmode actually tick otherwise no work. 
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ATestGroundGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const FString Filepath = FPaths::ProjectContentDir() + "Database/TestData.db";

	if (Db = new FSQLiteDatabase();Db->Open(*Filepath, ESQLiteDatabaseOpenMode::ReadWrite))
	{
		UE_LOG(LogTemp, Warning, TEXT("database is open"));
	}

	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogTemp, Warning, TEXT("init game works"));

}

void ATestGroundGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("prelogin works"));
	Super::PreLogin(Options, Address, UniqueId,ErrorMessage);
}


void ATestGroundGameMode::PostLogin(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("postlogin works"));
	Super::PostLogin(NewPlayer);

	MCharacter = Cast<ATestGroundCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestGroundCharacter::StaticClass()));
	if (!MCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("post login set the character to a new one "));
	}
}

void ATestGroundGameMode::Logout(AController* Exiting)
{
	UE_LOG(LogTemp, Warning, TEXT("number of players is: %d"), GetNumPlayers());
	if (GetNumPlayers() <= 1) // close database based on number of players. if the server shuts downthen fully close database. 
	{
		Db->Close();
		delete Db;
		UE_LOG(LogTemp, Warning, TEXT("closed db"));
	}
}

void ATestGroundGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ATestGroundGameMode::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogTemp, Warning, TEXT("called beginplayt on all actors"));
}


