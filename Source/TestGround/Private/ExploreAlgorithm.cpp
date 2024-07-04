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
#include "TestGround/TestGroundGameMode.h"
#include "Misc/DateTime.h"
#include "MySaveGame.h"
#include "NavigationSystem.h"

// Sets default values
AExploreAlgorithm::AExploreAlgorithm()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TotalNewVisits = 0;
	bcanAuto = true;
	bConfine = true;
}

void AExploreAlgorithm::SpawnDebugBoxForCell(FVector cell, bool bPersistentLines, float LifeTime, float Thickness, FColor color)
{
	//UE_LOG(LogTemp, Warning, TEXT("debug box is at: %s"), *cell.ToString());

	DrawDebugBox(GetWorld(), FVector(cell.X * 100, cell.Y * 100, cell.Z * 100 + 50), FVector(50.0f, 50.0f, 50.0f), color, bPersistentLines, LifeTime, 0, Thickness);

}

void AExploreAlgorithm::DrawAllBoxes()
{
	for (auto& box : VisitCount)
	{
		SpawnDebugBoxForCell(box.Key, true, 5.0, 1.0, FColor::Black);
	}
}

void AExploreAlgorithm::ExportTable()
{
	CurrentGameMode->PlayerTable->EmptyTable();
	VisitCount.ValueSort([](const int& A, const int& B) {
		return A < B; // sort by int value in the map
		});

	FDateTime CurrTime;
	
	for (auto& location : VisitCount)
	{
		counter++;
		FExplorationTable ET;
		ET.X = location.Key[0];
		ET.Y = location.Key[1];
		ET.Z = location.Key[2];

		ET.VisitCount = location.Value;
		if (CanTP.Contains(location.Key))
		{
			ET.bCanTeleport = CanTP[location.Key];
		}
		else
		{
			ET.bCanTeleport = true;
			UE_LOG(LogTemp, Warning, TEXT("YIKES"));
		}
		CurrentGameMode->PlayerTable->AddRow(FName(*FString::FromInt(counter)), ET);//add row to the datatable 
	}

}

void AExploreAlgorithm::ExportVisits()
{
	FCountTable VT;
	FDateTime CurrTime;
	//put this on a timer to keep track of number of new cells per exploration. 
	VT.numNewVisits = TotalNewVisits;
	VT.numSeconds = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	CurrentGameMode->CountTable->AddRow(FName(CurrTime.Now().ToString()), VT);
	UE_LOG(LogTemp, Warning, TEXT("ExportVisits"));
}

FVector AExploreAlgorithm::GetCurrentCell()
{
	FVector CellR;
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
		return Position;
	}

	return FVector::ZeroVector;
}

void AExploreAlgorithm::RecordCurrentState()
{
	FVector cell = GetCurrentCell();
	//UE_LOG(LogTemp, Warning, TEXT("///new key is, %s"), *key.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("///past state is, %s"), *PastState.ToString());
	//update visit count
	FHitResult* obstacles = nullptr;
	///
	if (cell != PastCell) //make sure the character has moved to a new location and the visit count only gets updated upon a new location. 
	{
		UMySaveGame* state = GetStateAsSave();
		//update Number of States saved per cell
		if (!StatesForCells.Contains(cell))
		{
			StatesForCells.Add(cell, TArray<UMySaveGame*>());
			SpawnDebugBoxForCell(cell, true, 3.0, 1.0, FColor::Blue);
		}
		StatesForCells[cell].Add(state);
		////
		if (!VisitCount.Contains(cell))
		{
			VisitCount.Add(cell, 1);
			TotalNewVisits++;
			//UE_LOG(LogTemp, Warning, TEXT("///visit count is %s, "), *cell.ToString());
		}
		else
		{
			VisitCount[cell]++;
			//UE_LOG(LogTemp, Warning, TEXT("///visit count is %d, "), VisitCount[cell]);
		}
		/////
		if (!CanTP.Contains(cell))
		{
			CanTP.Add(cell, true);
		}
		/////
		PastCell = cell;
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
	FVector selectedCell = FindLeastVisitedCell();

	if (StatesForCells.Contains(selectedCell)) 
	{
		TArray<UMySaveGame*>StateArray = StatesForCells[selectedCell];
		UMySaveGame* selectedState = StateArray[FMath::RandRange(0, StateArray.Num() - 1)];
		RestoreStateFromSave(selectedState,selectedCell);
		UE_LOG(LogTemp, Warning, TEXT("///teleport work and the visit count of this place is: %d"), VisitCount[selectedCell]);
		VisitCount[selectedCell]++;
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("///no work"));
	}
	
}

void AExploreAlgorithm::Randomize()
{
	Swap = UKismetMathLibrary::RandomIntegerInRange(0,10);
	dirnum = rand() % 4;
	Secondary_Input = UKismetMathLibrary::RandomIntegerInRange(0, 10);
	sr = rand() % 5;
}

FVector AExploreAlgorithm::FindLeastVisitedCell()
{
	VisitCount.ValueSort([](const int& A, const int& B) {
		 return A < B; // sort by int value in the map
	});

	for (auto& value : VisitCount)
	{
		if (CanTP.Contains(value.Key))
		{
			if (CanTP[value.Key] != false)
			{
				UE_LOG(LogTemp, Warning, TEXT("this is the value: %d"), value.Value);
				return value.Key;
			}
		}
	}

	return VisitCount.begin().Key();

}


UMySaveGame* AExploreAlgorithm::GetStateAsSave()
{
	UMySaveGame* GameData = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));

	GameData->Transform = TestCharacter->GetTransform();
	if (!TestCharacter->GetCharacterMovement()->Velocity.ContainsNaN())
	{
		GameData->Velocity = TestCharacter->GetVelocity();
	}

	return GameData;

}

void AExploreAlgorithm::RestoreStateFromSave(UMySaveGame* Save, FVector OldLocation)
{
	FHitResult* obstacles = new FHitResult(ForceInit);
	FVector RestoreOG = FVector(OldLocation.X * 100, OldLocation.Y * 100, OldLocation.Z * 100);
	if (TestCharacter->SetActorLocation(RestoreOG, false, obstacles, ETeleportType::TeleportPhysics))
	{
		TestCharacter->SetActorRotation(Save->Transform.GetRotation(), ETeleportType::TeleportPhysics);
		//if (!TestCharacter->GetCharacterMovement()->Velocity.ContainsNaN())
		//{
		//	//TestCharacter->GetCharacterMovement()->Velocity = Save->Velocity;
		//}

		//UE_LOG(LogTemp, Warning, TEXT("teleported here : %s"), *OldLocation.ToString());
		if (!CanTP.Contains(OldLocation))
		{
			CanTP.Add(OldLocation, true);
			//UE_LOG(LogTemp, Warning, TEXT("///added"));

		}
		else
		{
			CanTP[OldLocation] = true;
			//UE_LOG(LogTemp, Warning, TEXT("///is already in"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("failed teleport due to obstacle : %s"), *obstacles->GetActor()->GetActorLabel());
		if (!CanTP.Contains(OldLocation))
		{
			CanTP.Add(OldLocation, false);
			//UE_LOG(LogTemp, Warning, TEXT("///it is false"));
			SpawnDebugBoxForCell(OldLocation, true, 3.0, 1.0, FColor::Red);

		}
		else
		{
			CanTP[OldLocation] = false;
			//UE_LOG(LogTemp, Warning, TEXT("///it is false"));
			SpawnDebugBoxForCell(OldLocation, true, 3.0, 1.0, FColor::Red);
		}
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
	//write string to file\

	JsonString.ReplaceInline(TEXT("\r\n"), TEXT("")); //puts it on the same line. 
	JsonString.ReplaceInline(TEXT("\n"), TEXT(""));

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

	CurrentGameMode = Cast<ATestGroundGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (TestCharacter != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (bcanAuto)
		{
			TestCharacter = (GetWorld()->SpawnActor<ACharacter>(CharacterClass, this->GetActorLocation(), FRotator::ZeroRotator, SpawnParams));

			GetWorld()->GetFirstPlayerController()->SetPawn(TestCharacter);
			GetWorld()->GetFirstPlayerController()->Possess(TestCharacter);

			//timer for the fast tick which is the state saving
			FTimerHandle NewVisitTimer;
			GetWorld()->GetTimerManager().SetTimer(NewVisitTimer, this, &AExploreAlgorithm::ExportVisits, 1.0f, true);

			//timer for the slow tick which teleports the character to a random place
			FTimerHandle SlowTimer;
			GetWorld()->GetTimerManager().SetTimer(SlowTimer, this, &AExploreAlgorithm::Teleport, 4.0f, true);

			//timer for random input
			FTimerHandle RandomTimer;
			GetWorld()->GetTimerManager().SetTimer(RandomTimer, this, &AExploreAlgorithm::Randomize, 1.0f, true);

			PastCell = FVector(GetActorLocation().X / 100, GetActorLocation().Y / 100, GetActorLocation().Z / 100);

			StartingPoint = FVector(-1900,2500,200);

			Swap = true;
			Secondary_Input = false;

			dirnum = 0;
			sr = 0;

			counter++;
		}
		else
		{
			TestCharacter = Cast<ACharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), ACharacter::StaticClass()));
			GetWorld()->GetFirstPlayerController()->SetPawn(TestCharacter);
			GetWorld()->GetFirstPlayerController()->Possess(TestCharacter);

			//timer for the fast tick which is the state saving
			FTimerHandle NewVisitTimer;
			GetWorld()->GetTimerManager().SetTimer(NewVisitTimer, this, &AExploreAlgorithm::ExportVisits, 1.0f, true);

			PastCell = FVector(GetActorLocation().X / 100, GetActorLocation().Y / 100, GetActorLocation().Z / 100);

			StartingPoint = FVector(-1900, 2500, 200);

		}

		//Load in the movemeent component or whatever. 
	}

	VisitCount.Add(PastCell, 1);

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

	///Navmesh loading points and preloading them into the place. 
	NavMesh = FNavigationSystem::GetCurrent<UNavigationSystemV1>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (IsValid(NavMesh))
	{	/////get navmesh bounds and extent
		TSet<FNavigationBounds>bounds = NavMesh->GetNavigationBounds();
		FNavigationBounds area;
		for (const FNavigationBounds fnb : bounds)
		{
			area = fnb;
		}
		area.AreaBox.GetCenterAndExtents(BoundsCenter,BoundsExtent);
		UE_LOG(LogTemp, Warning, TEXT("box extents are : %s"), *BoundsExtent.ToString());
		UE_LOG(LogTemp, Warning, TEXT("box center  are : %s"), *BoundsCenter.ToString());
		//////////get random points from the navmesh
		UE_LOG(LogTemp, Warning, TEXT("Navmesh is loaded in"));
		FNavLocation ResultLocation;
		for (int i{ 0 }; i < 100; i++)
		{
			bool bSuccess = NavMesh->GetRandomPoint(ResultLocation);
			if (bSuccess)
			{
				VisitCount.Add((FVector(ResultLocation)/100), 1);
				CanTP.Add((FVector(ResultLocation) / 100), true);
			}
		}
		FVector CurrLoc = TestCharacter->GetActorLocation();
	}
	DrawAllBoxes();
	CurrentGameMode->CountTable->EmptyTable();
}

// Called every frame
void AExploreAlgorithm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RecordCurrentState();
	if (bcanAuto)
	{
		Search();
	}
	if (bConfine)
	{
		FVector CurrLoc = TestCharacter->GetActorLocation();
		if (!(CurrLoc.X < BoundsExtent.X + BoundsCenter.X && BoundsCenter.X - BoundsExtent.X < CurrLoc.X) || !(CurrLoc.Y < BoundsExtent.Y + BoundsCenter.Y && BoundsCenter.Y - BoundsExtent.Y < CurrLoc.Y) || !(CurrLoc.Z < BoundsExtent.Z + BoundsCenter.Z && BoundsCenter.Z - BoundsExtent.Z < CurrLoc.Z))
		{
			UE_LOG(LogTemp, Warning, TEXT("StartingPoint: %s"), *VisitCount.begin().Key().ToString());
			TestCharacter->SetActorLocation(VisitCount.begin().Key()*100);

		}
	}

}

