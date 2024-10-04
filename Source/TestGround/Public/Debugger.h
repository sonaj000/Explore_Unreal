// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Debugger.generated.h"

class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FVizTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int X;

	UPROPERTY(EditAnywhere)
	int Y;

	UPROPERTY(EditAnywhere)
	int Z;

	UPROPERTY(EditAnywhere)
	FString InputStrategy;

};

UCLASS()
class TESTGROUND_API ADebugger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADebugger();

	UPROPERTY()
	TArray<FString>TableNames;

	UPROPERTY(EditAnywhere)
	UStaticMesh* BothMaterial;

	UPROPERTY(EditAnywhere)
	UMaterial* MM;

	UPROPERTY(EditAnywhere)
	UInstancedStaticMeshComponent* InstancedMesh;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Exec)
	void DrawDebuggers();

	UPROPERTY(EditAnywhere)
	int NumCubes;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
