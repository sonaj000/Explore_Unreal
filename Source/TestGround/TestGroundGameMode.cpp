// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestGroundGameMode.h"
#include "TestGroundCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "SQLiteDatabase.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/FileHelper.h"
#include "Engine/DataTable.h"
#include "ExploreAlgorithm.h"

#include "MySaveGame.h"

ATestGroundGameMode::ATestGroundGameMode()
{
	// set default pawn class to our Blueprinted character

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

	if (PlayerTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("valid table"));
	}

	if (CountTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("valid count table"));
	}

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
	//PlayerTable->EmptyTable();
}

void ATestGroundGameMode::Logout(AController* Exiting)
{
	UE_LOG(LogTemp, Warning, TEXT("number of players is: %d"), GetNumPlayers());
	if (GetNumPlayers() <= 1) // close database based on number of players. if the server shuts downthen fully close database. 
	{
		Db->Close();
		delete Db;
		UE_LOG(LogTemp, Warning, TEXT("closed db"));
		FString MyFilePath = FPaths::ProjectContentDir(); //same thing as above delete later
		MyFilePath.Append(TEXT("TestData.csv")); //change this out to whatever csv file you are using for the data. 

		ExportData();
		//PlayerTable->EmptyTable(); //empties data for now, but we need to include export. 
	}
}

void ATestGroundGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ATestGroundGameMode::ExportData()
{
	FString Minute = FString::FromInt(counter) + "TestData2.csv";

	FString MyFilePath = FPaths::ProjectContentDir() + "DataAnalysis/"; //same thing as above delete later

	MyFilePath.Append(Minute); //change this out to whatever csv file you are using for the data. 
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile(); 
	/// Visit Table Vizualization
	FString VisitFile = FPaths::ProjectContentDir(); //same thing as above delete later
	VisitFile.Append(TEXT("NewVisit.csv")); //change this out to whatever csv file you are using for the data. 
	IPlatformFile& VF = FPlatformFileManager::Get().GetPlatformFile();
	counter++;
	///////////////
	if (FileManager.FileExists(*MyFilePath)) //check if the file exists
	{
		UE_LOG(LogTemp, Warning, TEXT("FilePaths: File found!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FilePaths: File not found!"));
	}
	AExploreAlgorithm* CurrAlg = Cast<AExploreAlgorithm>(UGameplayStatics::GetActorOfClass(GetWorld(), AExploreAlgorithm::StaticClass()));
	CurrAlg->ExportTable();
	FString TableString;
	if (PlayerTable != nullptr)
	{
		TableString = PlayerTable->GetTableAsCSV();
		FFileHelper::SaveStringToFile(TableString, *MyFilePath); //this will just immediately make a new file in the content dir with that same name as whatever u append to it. 
		UE_LOG(LogTemp, Warning, TEXT("we loaded correctly"));
	}
	////////////
	if (VF.FileExists(*VisitFile)) //check if the file exists
	{
		UE_LOG(LogTemp, Warning, TEXT("FilePaths: File found!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FilePaths: File not found!"));
	}
	FString VisitString;
	if (CountTable != nullptr)
	{
		VisitString = CountTable->GetTableAsCSV();
		FFileHelper::SaveStringToFile(VisitString, *VisitFile); //this will just immediately make a new file in the content dir with that same name as whatever u append to it. 
		UE_LOG(LogTemp, Warning, TEXT("we loaded visit correctly"));
	}
}

void ATestGroundGameMode::Draw()
{
	AExploreAlgorithm* CurrAlg = Cast<AExploreAlgorithm>(UGameplayStatics::GetActorOfClass(GetWorld(), AExploreAlgorithm::StaticClass()));
	CurrAlg->DrawAllBoxes();
}

void ATestGroundGameMode::Clear()
{
	AExploreAlgorithm* CurrAlg = Cast<AExploreAlgorithm>(UGameplayStatics::GetActorOfClass(GetWorld(), AExploreAlgorithm::StaticClass()));
	CurrAlg->FlushAllDebugs();
}


void ATestGroundGameMode::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogTemp, Warning, TEXT("called beginplayt on all actors"));
	FTimerHandle ExportTimer;
	GetWorld()->GetTimerManager().SetTimer(ExportTimer, this, &ATestGroundGameMode::ExportData, 60.0f, true);
}

void ATestGroundGameMode::P()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATestGroundCharacter* Po = Cast<ATestGroundCharacter>(GetWorld()->SpawnActor<ACharacter>(CharacterClass, MCharacter->GetActorLocation() + FVector(200,0,0), FRotator::ZeroRotator, SpawnParams));
	
	APlayerController* tp = GetWorld()->GetFirstPlayerController();
	tp->UnPossess();


	Po->AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	Po->AutoPossessPlayer = EAutoReceiveInput::Player0;

	tp->SetPawn(Po);
	tp->Possess(Po);
	Po->EnableInput(tp);
	//Po->BeginPlay();
}


