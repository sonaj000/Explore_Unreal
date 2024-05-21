// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ExploreAlgorithm.generated.h"

class ATestGroundGameMode;
class UMySaveGame;
class ACharacter;
class APlayerController;
class FJsonObject;

USTRUCT(BlueprintType, Category = "Print")
struct FTestStruct
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
	FString MyString = "Hello Jason"; //need uproperty to show up in json file

	UPROPERTY(EditAnywhere)
	int counter = 2; //need uproperty to show up in json file


};


UCLASS()
class TESTGROUND_API AExploreAlgorithm : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExploreAlgorithm();

	UPROPERTY(EditAnywhere)
	int NumSteps;

	UPROPERTY()
	int TeleportInterval;

	UPROPERTY(EditAnywhere, Category = "Character") 
	TSubclassOf<ACharacter>CharacterClass; //the character selection.

	UPROPERTY(EditAnywhere, Category = "Character")
	ACharacter* TestCharacter; //the character selection.

	UPROPERTY(EditAnywhere, Category = "Character")
	APlayerController* CharacterController; //the character selection.

public:

	TMap<FVector, TArray<UMySaveGame*>>StatesForCells;

	TMap<FVector, int>VisitCount;

	UFUNCTION()
	void SpawnDebugBoxForCell(FVector cell, bool bPersistentLines,float LifeTime, float Thickness);

protected: //Algorithm Functions

	UFUNCTION()
	TArray<FVector>GetCellState();

	void RecordCurrentState();

	UFUNCTION()
	void Search();

	UFUNCTION()
	void Teleport();

	void Randomize();

	UPROPERTY()
	int Swap;

	UPROPERTY()
	int Secondary_Input;

	UPROPERTY()
	int dirnum;

	UPROPERTY()
	int sr;

	UPROPERTY()
	FVector PastState;

	UPROPERTY()
	TArray<FVector>Directions;

	FVector LeastVisited();

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

	static TSharedPtr<FJsonObject>ReadJson(FString JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage);

	static void WriteJson(FString JsonFilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess, FString& OutInfoMessage);

	UFUNCTION()
	static FTestStruct ReadStructFromJson(FString JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage);
	
	UFUNCTION()
	static void WriteStructToJsonFile(FString JsonFilePath, FTestStruct Struct, bool& bOutSuccess, FString& OutInfoMessage);

	UFUNCTION(Exec)
	void T();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
