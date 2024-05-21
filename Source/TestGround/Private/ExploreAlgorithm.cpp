// Fill out your copyright notice in the Description page of Project Settings.


#include "ExploreAlgorithm.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"
#include "MyCharacterMovementComponent.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "JsonObjectConverter.h"

#include "MySaveGame.h"

// Sets default values
AExploreAlgorithm::AExploreAlgorithm()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AExploreAlgorithm::SpawnDebugBoxForCell(FVector cell, bool bPersistentLines, float LifeTime, float Thickness)
{
	//UE_LOG(LogTemp, Warning, TEXT("debug box is at: %s"), *cell.ToString());

	DrawDebugBox(GetWorld(), FVector(cell.X * 100, cell.Y * 100, cell.Z * 100 + 50), FVector(50.0f, 50.0f, 50.0f), FColor::Red, bPersistentLines, LifeTime, 0, Thickness);
}

TArray<FVector>AExploreAlgorithm::GetCellState()
{
	TArray<FVector>CellR;
	if (TestCharacter != nullptr)
	{
		int x = TestCharacter->GetActorLocation().X / 100;
		int y = TestCharacter->GetActorLocation().Y / 100;
		int z = TestCharacter->GetActorLocation().Z / 100;

		if (z < 0)
		{
			z = 1;
		}

		FVector Position = FVector(x, y, z);

		CellR.Add(Position);
	}

	return CellR;
}

void AExploreAlgorithm::RecordCurrentState()
{
	TArray<FVector>holder = GetCellState();
	FVector key = holder[0];
	//UE_LOG(LogTemp, Warning, TEXT("///new key is, %s"), *key.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("///past state is, %s"), *PastState.ToString());
	//update visit count
	if (key != PastState) //make sure the character has moved to a new location and the visit count only gets updated upon a new location. 
	{
		UMySaveGame* value = GetStateAsSave();
		//update Number of States saved per cell
		if (!StatesForCells.Contains(key))
		{
			StatesForCells.Add(key, TArray<UMySaveGame*>());
			SpawnDebugBoxForCell(key, true, 3.0, 1.0);
		}
		StatesForCells[key].Add(value);

		if (!VisitCount.Contains(key))
		{
			//UE_LOG(LogTemp, Warning, TEXT("///adding new key "));
			VisitCount.Add(key, 1);
		}
		else
		{
			VisitCount[key]++;
			UE_LOG(LogTemp, Warning, TEXT("///visit count is %d, "), VisitCount[key]);
		}
		PastState = key;
	}
}

void AExploreAlgorithm::Search()
{
	if (Swap < 6)
	{
		TestCharacter->AddMovementInput(Directions[dirnum]);
		//TestCharacter
		//make a new random input
	}
	if (Secondary_Input < 7)
	{

		if (sr == 4)
		{
			TestCharacter->Jump();
		}
		else
		{
			TestCharacter->AddMovementInput(Directions[sr]);
		}

	}

}

void AExploreAlgorithm::Teleport()
{
	FVector selectedCell = LeastVisited();

	if (StatesForCells.Contains(selectedCell)) 
	{
		TArray<UMySaveGame*>StateArray = StatesForCells[selectedCell];
		UMySaveGame* selectedState = StateArray[FMath::RandRange(0, StateArray.Num() - 1)];
		RestoreStateFromSave(selectedState,selectedCell);
		UE_LOG(LogTemp, Warning, TEXT("///teleport work"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("///no work"));
	}
	
}

void AExploreAlgorithm::Randomize()
{
	Swap = UKismetMathLibrary::RandomIntegerInRange(0,10);
	dirnum = rand() % 4;
	Secondary_Input = UKismetMathLibrary::RandomIntegerInRange(0, 10);
	sr = rand() % 5;
}

FVector AExploreAlgorithm::LeastVisited()
{
	VisitCount.ValueSort([](const int& A, const int& B) {
		 return A < B; // sort by int value in the map
	});
	int lowestval = VisitCount.begin().Value();
	UE_LOG(LogTemp, Warning, TEXT("this is the key: %d"),VisitCount.begin().Value());
	return VisitCount.begin().Key();

	//float longestDist = 0.0f;
	//FVector longestVector = VisitCount.begin().Key();
	//FVector Currloc = TestCharacter->GetActorLocation();
	//for (auto& elem : VisitCount)
	//{
	//	//UE_LOG(LogTemp, Warning, TEXT("value: %d"), elem.Value);
	//	if (elem.Value > lowestval)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("returned from lowestval"));
	//		return longestVector;
	//	}
	//	float currd = FVector::Dist(elem.Key, Currloc);
	//	if (currd > longestDist)
	//	{
	//		longestDist = currd;
	//		longestVector = elem.Key;
	//	}
	//}

	////for (auto& Elem : VisitCount)
	////{
	////	UE_LOG(LogTemp, Warning, TEXT("we are starting with key: %s || value :%d"), *Elem.Key.ToString(), Elem.Value);
	////}

	////UE_LOG(LogTemp, Warning, TEXT("this is the key: %s"),*VisitCount.begin().Key().ToString());
	//return longestVector;
}

UMySaveGame* AExploreAlgorithm::GetStateAsSave()
{
	UMySaveGame* GameData = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));

	GameData->Transform = TestCharacter->GetTransform();
	GameData->Velocity = TestCharacter->GetVelocity();

	return GameData;

}

void AExploreAlgorithm::RestoreStateFromSave(UMySaveGame* Save, FVector OldLocation)
{
	FHitResult* obstacles = nullptr;
	FVector RestoreOG = FVector(OldLocation.X * 100, OldLocation.Y * 100, OldLocation.Z * 100 + 50);
	if (TestCharacter->SetActorLocation(RestoreOG, true, obstacles, ETeleportType::TeleportPhysics))
	{
		TestCharacter->SetActorRotation(Save->Transform.GetRotation(), ETeleportType::TeleportPhysics);
		TestCharacter->GetCharacterMovement()->Velocity = Save->Velocity;


		UE_LOG(LogTemp, Warning, TEXT("teleported here : %s"), *OldLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("failed teleport due to obstacle"));
	}
}

FString AExploreAlgorithm::ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (FileManager.FileExists(*FilePath)) //check if the file exists
	{
		UE_LOG(LogTemp, Warning, TEXT("FilePaths: File found!"));
	}
	else
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("file not found %s"), *FilePath);
		UE_LOG(LogTemp, Warning, TEXT("FilePaths: File not found!"));
		return " ";
	}

	FString FileContent;
	//Read the csv file
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read String failed %s"), *FilePath);
		return "";
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read String succeeded - %s"), *FilePath);

	return FileContent;
}

void AExploreAlgorithm::WriteStringToFile(FString FilePath, FString String, bool& bOutSuccess, FString& OutInfoMessage)
{
	if (!FFileHelper::SaveStringToFile(String, *FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Write String To File Failed: %s"), *FilePath);
		return;
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write String To File Succeeded - %s"), *FilePath);

}

TSharedPtr<FJsonObject> AExploreAlgorithm::ReadJson(FString JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	FString JsonString = ReadStringFromFile(JsonFilePath, bOutSuccess, OutInfoMessage);
	if (!bOutSuccess)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObject;

	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), JsonObject))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read Json Failed - %s"), *JsonString);
		return nullptr;
	}
	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read Json Succeeded - %s"), *JsonFilePath);
	return JsonObject;
}

void AExploreAlgorithm::WriteJson(FString JsonFilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess, FString& OutInfoMessage)
{
	FString JsonString;

	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString,0)))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("write json failed"));
		return;
	}
	//write string to file
	WriteStringToFile(JsonFilePath, JsonString, bOutSuccess, OutInfoMessage);
	if (!bOutSuccess)
	{
		return;
	}
	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write Json Succeeded -%s"), *JsonFilePath);
}

FTestStruct AExploreAlgorithm::ReadStructFromJson(FString JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	TSharedPtr<FJsonObject>JsonObject = ReadJson(JsonFilePath, bOutSuccess, OutInfoMessage);
	if (!bOutSuccess)
	{
		return FTestStruct();
	}
	
	FTestStruct FirstTest;
	if (!FJsonObjectConverter::JsonObjectToUStruct<FTestStruct>(JsonObject.ToSharedRef(), &FirstTest))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read struct Json Failed - %s"), *JsonFilePath);
		return FTestStruct();
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read struct Json succeeded - %s"), *JsonFilePath);
	return FirstTest;
}

void AExploreAlgorithm::WriteStructToJsonFile(FString JsonFilePath, FTestStruct Struct, bool& bOutSuccess, FString& OutInfoMessage)
{
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Struct);
	if (JsonObject == nullptr)
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("big yikes failed to write struct"));
		return;
	}
	WriteJson(JsonFilePath, JsonObject, bOutSuccess, OutInfoMessage);
}

void AExploreAlgorithm::T()
{
	FString M = FPaths::ProjectContentDir(); //same thing as above delete later
	M.Append(TEXT("DataAnalysis/TestJson.json")); //change this out to whatever csv file you are using for the data. 

	FTestStruct F;
	F.MyString = "yay";
	bool b;
	FString t = "";

	WriteStructToJsonFile(M, F, b, t);
	UE_LOG(LogTemp, Warning, TEXT("T worked"));

}

// Called when the game starts or when spawned
void AExploreAlgorithm::BeginPlay()
{
	Super::BeginPlay();

	if (TestCharacter != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TestCharacter = (GetWorld()->SpawnActor<ACharacter>(CharacterClass, this->GetActorLocation(), FRotator::ZeroRotator, SpawnParams));
		GetWorld()->GetFirstPlayerController()->Possess(TestCharacter);

		CharacterController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

		//timer for the fast tick which is the state saving
		//FTimerHandle FastTimer;
		//GetWorld()->GetTimerManager().SetTimer(FastTimer, this, &AExploreAlgorithm::RecordCurrentState, 0.1f, true);

		//timer for the slow tick which teleports the character to a random place
		FTimerHandle SlowTimer;
		GetWorld()->GetTimerManager().SetTimer(SlowTimer, this, &AExploreAlgorithm::Teleport, 4.0f, true);

		//timer for random input
		FTimerHandle RandomTimer;
		GetWorld()->GetTimerManager().SetTimer(RandomTimer, this, &AExploreAlgorithm::Randomize, 1.0f, true);

		PastState = FVector(GetActorLocation().X / 100, GetActorLocation().Y / 100, GetActorLocation().Z / 100);

		Swap = true;
		Secondary_Input = false;

		dirnum = 0;
		sr = 0;

		//Load in the movemeent component or whatever. 
	}

	VisitCount.Add(PastState, 1);

	//add the random search. 
	FVector Forward = FVector(0.0, -1.0, 0.0f);
	FVector Backward = FVector(0.0, 1.0f, 0.0f);
	FVector Right = FVector(1.0, 0.0, 0.0f);
	FVector Left = FVector(-1.0, 0.0, 0.0f);

	Directions.Add(Forward);
	Directions.Add(Backward);
	Directions.Add(Right);
	Directions.Add(Left);

	T();
	//LeastVisited();
	
}

// Called every frame
void AExploreAlgorithm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RecordCurrentState();
	Search();

}

