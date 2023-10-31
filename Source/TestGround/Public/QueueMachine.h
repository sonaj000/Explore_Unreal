// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QueueMachine.generated.h"

class UBoxComponent;
class UTestWidget;
UCLASS()
class TESTGROUND_API AQueueMachine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AQueueMachine();

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UTestWidget>TestClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTestWidget* Widget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	bool bCanCollide;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
