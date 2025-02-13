// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"

#include "ExploreAlgorithm.generated.h"

class ATestGroundGameMode;
class ATestGroundCharacter;
class UMySaveGame;
class ACharacter;
class APlayerController;
class FJsonObject;
class UNavigationSystemV1;
class UBoxComponent;


USTRUCT(BlueprintType)
struct FExplorationTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int X;

	UPROPERTY(EditAnywhere)
	int Y;

	UPROPERTY(EditAnywhere)
	int Z;

	UPROPERTY(EditAnywhere)
	int VisitCount;

	UPROPERTY(EditAnywhere)
	bool bCanTeleport;

};

USTRUCT(BlueprintType)
struct FCountTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int numNewVisits;

	UPROPERTY(EditAnywhere)
	double numSeconds;

};

UENUM()
enum InputType
{
	vanilla,
	stateful,
	brownian_motion
};

UCLASS()
class TESTGROUND_API AExploreAlgorithm : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExploreAlgorithm();

	UPROPERTY(EditAnywhere)
	bool bcanAuto;

	UPROPERTY(EditAnywhere)
	int NumSteps;

	UPROPERTY(EditAnywhere)
	int TeleportInterval;

	UPROPERTY(EditAnywhere)
	float CellSize;

	UPROPERTY(EditAnywhere)
	bool bNoHTP;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<InputType>CurrentInput;

	UPROPERTY(EditAnywhere, Category = "Character") 
	TSubclassOf<ACharacter>CharacterClass; //the character selection.

	UPROPERTY(EditAnywhere, Category = "Character")
	ATestGroundCharacter* TestCharacter; //the character selection.

	UPROPERTY(EditAnywhere, Category = "Character")
	APlayerController* CharacterController; //the character selection.

private: //command line parsing

	void CMDParse();

public:

	TMap<FVector, TArray<UMySaveGame*>>StatesForCells;

	TMap<FVector, int>VisitCount;

	TMap<FVector, bool>CanTP;

	UFUNCTION()
	void SpawnDebugBoxForCell(FVector cell, bool bPersistentLines,float LifeTime, float Thickness, FColor color);

	UFUNCTION()
	void FlushAllDebugs();

	UFUNCTION()
	void DrawAllBoxes();

	UPROPERTY()
	int counter;

	UFUNCTION()
	void ExportTable();

	UPROPERTY(EditAnywhere)
	bool bConfine;

	UPROPERTY(EditAnywhere)
	int TimeDilation;

	UPROPERTY(EditAnywhere)
	bool bEnableHeadless;

protected:
	UFUNCTION()
	void NavMeshSeeding(int NumSeeds);

	UPROPERTY(EditAnywhere, Category = "Bounds")
	UBoxComponent* ConfineBounds;

public:

	UPROPERTY(EditAnywhere)
	int numSeeds;

	UPROPERTY(EditAnywhere)
	UNavigationSystemV1* NavMesh;

	UPROPERTY(VisibleAnywhere)
	FVector BoundsExtent;

	UPROPERTY(VisibleAnywhere)
	FVector BoundsCenter;

protected: //Algorithm Functions

	UFUNCTION()
	FVector GetCurrentCell();

	void RecordCurrentState();

	UFUNCTION()
	void Search(float DeltaTime);

	UFUNCTION()
	void Teleport();

	void Randomize();

	UFUNCTION()
	int Vanilla();

	UFUNCTION()
	int Stateful();

	UPROPERTY()
	int bitvalue;

	UFUNCTION()
	FVector BrownianMotion();

	FVector BMinput;

	UPROPERTY(EditAnywhere)
	int BMStepSize;

	UFUNCTION()
	float GaussianSampling(float mean, float variance);

	UPROPERTY()
	int Swap;

	UPROPERTY()
	int Secondary_Input;

	UPROPERTY()
	int dirnum;

	UPROPERTY()
	int sr;

	UPROPERTY()
	FVector PastCell;

	UPROPERTY()
	FVector StartingPoint;

	UPROPERTY()
	TArray<FVector>Directions;

	FVector FindLeastVisitedCell();

	UPROPERTY(VisibleAnywhere)
	int TotalNewVisits;

	UFUNCTION()
	void ExportVisits();

private: //Save State
	UFUNCTION()
	UMySaveGame* GetStateAsSave();

	UFUNCTION()
	void RestoreStateFromSave(UMySaveGame* Save, FVector OldLocation);

	UPROPERTY()
	ATestGroundGameMode* CurrentGameMode;

	
public:
	UFUNCTION(BlueprintCallable, Category = "Read Write File")
	static FString ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);
	
	UFUNCTION(BlueprintCallable, Category = "Read Write File")
	static void WriteStringToFile(FString FilePath, FString String, bool& bOutSuccess, FString& OutInfoMessage);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
