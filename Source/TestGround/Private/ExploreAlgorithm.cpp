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
#include "InputActionValue.h"
#include "TestGround/TestGroundCharacter.h"
#include "Components/BoxComponent.h"


// Sets default values
AExploreAlgorithm::AExploreAlgorithm()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	ConfineBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Confine Bounds"));

	PrimaryActorTick.bCanEverTick = true;
	TotalNewVisits = 0;
	bcanAuto = true;
	bConfine = true;
	numSeeds = 100;
	TimeDilation = 2.0f;
	bEnableHeadless = false;
	TeleportInterval = 5;
	CellSize = 1.0;
	BMStepSize = 1;
	BMinput = FVector::ZeroVector;
	CurrentInput = InputType::vanilla;
	bitvalue = rand() % (1 << 5);
	bNoHTP = false;
	CMDParse();
}

void AExploreAlgorithm::CMDParse()
{
	UE_LOG(LogTemp, Warning, TEXT("PARSED THE COMMANDLINE INPUTS FROM THE BEGINNING"));
	//headless, time dilation, numseeds to seed the thing,input style,cell size
	FString HeadlessValue;
	if (FParse::Value(FCommandLine::Get(), TEXT("headless="), HeadlessValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("there is a headless value"));
		// Convert the string to lowercase for case-insensitive comparison
		HeadlessValue = HeadlessValue.ToLower();

		// Convert string to boolean
		if (HeadlessValue == "true")
		{
			bEnableHeadless = true;
		}
		else if (HeadlessValue == "false")
		{
			bEnableHeadless = false;
		}

	}

	FString TimeValue;
	if (FParse::Value(FCommandLine::Get(), TEXT("timedilation="), TimeValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("there is a timedilation value"));
		// Convert the string to lowercase for case-insensitive comparison
		TimeValue = TimeValue.ToLower();
		TimeDilation = FCString::Atof(*TimeValue);
	}

	FString SeedValue;
	if (FParse::Value(FCommandLine::Get(), TEXT("numseed="), SeedValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("there is a numseed value"));
		// Convert the string to lowercase for case-insensitive comparison
		SeedValue = SeedValue.ToLower();
		numSeeds = FCString::Atoi(*SeedValue);
	}

	FString InputStrategy;
	if (FParse::Value(FCommandLine::Get(), TEXT("input_strategy="), InputStrategy))
	{
		UE_LOG(LogTemp, Warning, TEXT("there is a valid input strategy"));
		// Convert the string to lowercase for case-insensitive comparison
		InputStrategy = InputStrategy.ToLower();
		if (InputStrategy == "brownian_motion")
		{
			UE_LOG(LogTemp, Warning, TEXT("is brownian"));
			CurrentInput = InputType::brownian_motion;
		}
		else if (InputStrategy == "stateful")
		{
			UE_LOG(LogTemp, Warning, TEXT("is stateful"));
			CurrentInput = InputType::stateful;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("is vanilla"));
			CurrentInput = InputType::vanilla;
		}
	}

	FString CSV;
	if (FParse::Value(FCommandLine::Get(), TEXT("cellsize="), CSV))
	{
		UE_LOG(LogTemp, Warning, TEXT("there is a cell size value"));
		// Convert the string to lowercase for case-insensitive comparison
		CSV = CSV.ToLower();
		CellSize = FCString::Atof(*CSV);
	}

}

void AExploreAlgorithm::SpawnDebugBoxForCell(FVector cell, bool bPersistentLines, float LifeTime, float Thickness, FColor color)
{
	//UE_LOG(LogTemp, Warning, TEXT("debug box is at: %s"), *cell.ToString());

	DrawDebugBox(GetWorld(), FVector(cell.X * 100, cell.Y * 100, cell.Z * 100 + (50)), FVector(50.0f,50.0f, 50.0f), color, bPersistentLines, LifeTime, 0, Thickness);

}

void AExploreAlgorithm::FlushAllDebugs()
{
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
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

void AExploreAlgorithm::NavMeshSeeding(int NumSeeds)
{
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
		//area.AreaBox.GetCenterAndExtents(BoundsCenter, BoundsExtent);
		//UE_LOG(LogTemp, Warning, TEXT("box extents are : %s"), *BoundsExtent.ToString());
		//UE_LOG(LogTemp, Warning, TEXT("box center  are : %s"), *BoundsCenter.ToString());
		//////////get random points from the navmesh
		UE_LOG(LogTemp, Warning, TEXT("Navmesh is loaded in"));
		FNavLocation ResultLocation;
		for (int i{ 0 }; i < NumSeeds; i++)
		{
			bool bSuccess = NavMesh->GetRandomPoint(ResultLocation);
			if (bSuccess)
			{
				VisitCount.Add((FVector(ResultLocation) / (100)), 0);
				CanTP.Add((FVector(ResultLocation) / (100)), true);
				StatesForCells.Add((FVector(ResultLocation) / (100)), TArray<UMySaveGame*>());
				StatesForCells[(FVector(ResultLocation) / (100))].Add(GetStateAsSave());
			}
		}
	}
	BoundsExtent = ConfineBounds->GetScaledBoxExtent();
	BoundsCenter = GetActorLocation();
}

void AExploreAlgorithm::ExportVisits()
{
	FCountTable VT;
	FDateTime CurrTime;
	//put this on a timer to keep track of number of new cells per exploration. 
	VT.numNewVisits = TotalNewVisits;
	VT.numSeconds = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	CurrentGameMode->CountTable->AddRow(FName(CurrTime.Now().ToString()), VT);
	//UE_LOG(LogTemp, Warning, TEXT("ExportVisits"));
}

FVector AExploreAlgorithm::GetCurrentCell()
{
	FVector CellR;
	if (TestCharacter != nullptr)
	{ 
		int x = TestCharacter->GetActorLocation().X / (100);
		int y = TestCharacter->GetActorLocation().Y / (100);
		int z = TestCharacter->GetActorLocation().Z / (100);

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
			//SpawnDebugBoxForCell(cell, true, 3.0, 1.0, FColor::Blue);
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

void AExploreAlgorithm::Search(float Deltatime)
{
	const int F = 1 << 0;  // 0001 in binary, represents "up"
	const int B = 1 << 1;  // 0010 in binary, represents "right"
	const int R = 1 << 2;  // 0100 in binary, represents "left"
	const int L = 1 << 3;  // 1000 in binary, represents "down"
	const int J = 1 << 4;

	if (CurrentInput == InputType::vanilla || CurrentInput == InputType::stateful)
	{
		if (bitvalue & F)
		{
			FInputActionValue InputValue(FVector(0.0, -1.0, 0.0f));
			TestCharacter->Move(InputValue);
		}
		if (bitvalue & B)
		{
			FInputActionValue InputValue(FVector(0.0, 1.0f, 0.0f));
			TestCharacter->Move(InputValue);
		}
		if (bitvalue & R)
		{
			FInputActionValue InputValue(FVector(1.0, 0.0, 0.0f));
			TestCharacter->Move(InputValue);
		}
		if (bitvalue & L)
		{
			FInputActionValue InputValue(FVector(-1.0, 0.0, 0.0f));
			TestCharacter->Move(InputValue);
		}
		if (bitvalue & J)
		{
			TestCharacter->Jump();
		}

	}
	else if (CurrentInput == InputType::brownian_motion)
	{
		FInputActionValue InputValue(BMinput);
		TestCharacter->Move(InputValue);
		if (BMinput.Z > 1)
		{
			TestCharacter->Jump();
		}
	}

}

void AExploreAlgorithm::Teleport()
{
	FVector selectedCell;
	if (!bNoHTP) //we want a heuristic
	{
		UE_LOG(LogTemp, Warning, TEXT("///heuristic"));
		selectedCell = FindLeastVisitedCell();
	}
	else
	{
		TArray<FVector>keys;
		VisitCount.GetKeys(keys);
		selectedCell = keys[FMath::RandRange(0, keys.Num() - 1)];
		UE_LOG(LogTemp, Warning, TEXT("/// no heuristic"));
	}
	if (StatesForCells.Contains(selectedCell)) 
	{
		TArray<UMySaveGame*>StateArray = StatesForCells[selectedCell];
		UMySaveGame* selectedState = StateArray[FMath::RandRange(0, StateArray.Num() - 1)]; 
		if (selectedState == nullptr)
		{
			StateArray.Remove(selectedState);
			selectedState = StateArray[FMath::RandRange(0, StateArray.Num() - 1)];
		}
		RestoreStateFromSave(selectedState,selectedCell);
		VisitCount[selectedCell]++;
		UE_LOG(LogTemp, Warning, TEXT("///teleport work and the visit count of this place is: %d"), VisitCount[selectedCell]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("///no work"));
	}
	
}

void AExploreAlgorithm::Randomize()
{
	Vanilla();
	BMinput = BrownianMotion();
}

int AExploreAlgorithm::Vanilla()
{
	bitvalue = rand() % (1 << 5);
	return bitvalue;
}

int AExploreAlgorithm::Stateful()
{
	bitvalue ^= (1 << (rand() % 5));
	return bitvalue;
}

FVector AExploreAlgorithm::BrownianMotion()
{
	FVector Step = FVector(GaussianSampling(0,1), GaussianSampling(0,1), GaussianSampling(0,1));
	UE_LOG(LogTemp, Warning, TEXT("///step is :%s"), *Step.ToString());
	FVector WeinerResult = Step * BMStepSize;
	return WeinerResult;
}

float AExploreAlgorithm::GaussianSampling(float mean, float variance)
{
	// Box-Muller transform to generate a standard normal distribution (mean = 0, variance = 1)
	float U1 = FMath::FRand(); // Uniform random number between 0 and 1
	float U2 = FMath::FRand(); // Uniform random number between 0 and 1

	// Apply Box-Muller formula
	float Z0 = FMath::Sqrt(-2.0f * FMath::Loge(U1)) * FMath::Cos(2.0f * PI * U2);


	// Adjust for the desired mean and variance (standard deviation)
	float StandardDeviation = FMath::Sqrt(variance); // Standard deviation is the square root of variance
	float r = mean + Z0 * StandardDeviation;   // Scale Z0 by standard deviation and shift by the mean

	return r; // This is a normally distributed random number
	
}


FVector AExploreAlgorithm::FindLeastVisitedCell()
{
	VisitCount.ValueSort([](const int& A, const int& B) {
		 return A < B; // sort by int value in the map
	});

	//for (auto& value : VisitCount)
	//{
	//	if (CanTP.Contains(value.Key))
	//	{
	//		if (CanTP[value.Key] != false)
	//		{
	//			UE_LOG(LogTemp, Warning, TEXT("this is the value: %d"), value.Value);
	//			return value.Key;
	//		}
	//	}
	//}

	return VisitCount.begin().Key();

}


UMySaveGame* AExploreAlgorithm::GetStateAsSave()
{
	UMySaveGame* GameData = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));

	GameData->Transform = TestCharacter->GetTransform();
	if (!TestCharacter->GetCharacterMovement()->Velocity.ContainsNaN())
	{
		GameData->Velocity = TestCharacter->GetVelocity();
		GameData->BitValue = bitvalue;
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
		bitvalue = Save->BitValue;
		//if (!TestCharacter->GetCharacterMovement()->Velocity.ContainsNaN())
		//{
		//	//TestCharacter->GetCharacterMovement()->Velocity = Save->Velocity;
		//}

		//UE_LOG(LogTemp, Warning, TEXT("teleported here : %s"), *OldLocation.ToString());
		if (!CanTP.Contains(OldLocation))
		{
			CanTP.Add(OldLocation, true);

		}
		else
		{
			CanTP[OldLocation] = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("failed teleport due to obstacle : %s"), *obstacles->GetActor()->GetActorLabel());
		if (!CanTP.Contains(OldLocation))
		{
			CanTP.Add(OldLocation, false);

		}
		else
		{
			CanTP[OldLocation] = false;
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


// Called when the game starts or when spawned
void AExploreAlgorithm::BeginPlay()
{
	Super::BeginPlay();
	CMDParse();

	CurrentGameMode = Cast<ATestGroundGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (bEnableHeadless)
	{
		if (GEngine)
		{
			GEngine->GameViewport->bDisableWorldRendering = true;
			//GEngine->GameViewport->bdi
		}
	}

	if (TestCharacter != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (bcanAuto)
		{
			TestCharacter = (GetWorld()->SpawnActor<ATestGroundCharacter>(CharacterClass, this->GetActorLocation(), FRotator::ZeroRotator, SpawnParams));
			TestCharacter->CustomTimeDilation = TimeDilation;
			GetWorld()->GetFirstPlayerController()->SetPawn(TestCharacter);
			GetWorld()->GetFirstPlayerController()->Possess(TestCharacter);

			//timer for the fast tick which is the state saving
			FTimerHandle NewVisitTimer;
			GetWorld()->GetTimerManager().SetTimer(NewVisitTimer, this, &AExploreAlgorithm::ExportVisits, 1.0f, true);

			//timer for the slow tick which teleports the character to a random place
			FTimerHandle SlowTimer;
			GetWorld()->GetTimerManager().SetTimer(SlowTimer, this, &AExploreAlgorithm::Teleport, TeleportInterval, true);

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
			TestCharacter = Cast<ATestGroundCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), ACharacter::StaticClass()));
			TestCharacter->CustomTimeDilation = TimeDilation;
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

	Directions.Add(FVector(1, 0, 0));


	NavMeshSeeding(numSeeds);
	FVector CurrLoc = TestCharacter->GetActorLocation();
	DrawAllBoxes();
	CurrentGameMode->CountTable->EmptyTable();
	UE_LOG(LogTemp, Warning, TEXT("timedilation is set in beginplay: %d"), TimeDilation);
	UE_LOG(LogTemp, Warning, TEXT("enableheadless in beginpaly is %s"), (bEnableHeadless ? TEXT("true") : TEXT("false")));
	UE_LOG(LogTemp, Warning, TEXT("numseeds is set in beginplay: %d"), numSeeds);

}

// Called every frame
void AExploreAlgorithm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RecordCurrentState();
	if (bcanAuto)
	{
		Search(DeltaTime);
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

