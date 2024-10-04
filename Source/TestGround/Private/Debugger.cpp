// Fill out your copyright notice in the Description page of Project Settings.


#include "Debugger.h"
#include "ExploreAlgorithm.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Components/InstancedStaticMeshComponent.h"



// Sets default values
ADebugger::ADebugger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	NumCubes = 1000;
	BothMaterial = CreateDefaultSubobject<UStaticMesh>(TEXT("Static Mesh"));
	//MM = CreateDefaultSubobject<UMaterial>(TEXT("Start Material"));
	InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Instanced Static Mesh"));
	InstancedMesh->SetWorldLocation(FVector::ZeroVector); // Place the component at world origin
	InstancedMesh->SetAbsolute(true, true, true);  // Ignore parent actor's location, rotation, and scale

}

// Called when the game starts or when spawned
void ADebugger::BeginPlay()
{
	Super::BeginPlay();
	DrawDebuggers();
	this->SetActorLocation(FVector::ZeroVector);
}


void ADebugger::DrawDebuggers()
{
	if (BothMaterial)
	{
		InstancedMesh->SetStaticMesh(BothMaterial); // Set the mesh to your cube static mesh
		InstancedMesh->RegisterComponent(); // Register it with the actor
		InstancedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	FString FilePath = TEXT("/Game/Data/Combined.Combined");
	UDataTable* MyDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *FilePath));
	if (MyDataTable)
	{
		UE_LOG(LogTemp, Log, TEXT("DataTable loaded successfully!"));
		// Get all row names from the DataTable
		TArray<FName> RowNames = MyDataTable->GetRowNames();

		for (const FName& RowName : RowNames)
		{
			// Find the row using its name and cast it to FMyDataRow
			FVizTable* Row = MyDataTable->FindRow<FVizTable>(RowName, TEXT(""));
			if (Row)
			{
				FVector Location = FVector(Row->X * 100, Row->Y * 100, Row->Z * 100 + 50); // Your function to get random positions
				FTransform Transform(FRotator(180,360,0), Location, FVector(1.0f));
				int32 InstanceIndex = InstancedMesh->AddInstance(Transform);
				if (Row->InputStrategy == "Both")
				{
					InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 0.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 1, 0.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 2, 1.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 3, 0.05f);

				}
				else if (Row->InputStrategy == "Manual")
				{

					InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 1.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 1, 0.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 2, 0.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 3, 0.1f);

				}
				else
				{
					InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 0.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 1, 1.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 2, 0.0f);
					InstancedMesh->SetCustomDataValue(InstanceIndex, 3, 0.1f);
				}

			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable."));
	}

}


// Called every frame
void ADebugger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

