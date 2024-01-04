// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PressurePlate.generated.h"

class UBoxComponent;
UCLASS()
class TESTGROUND_API APressurePlate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APressurePlate();

	UPROPERTY(BlueprintReadOnly)
	bool bIsPressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Bridge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor>BridgeActor;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* Tile;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* HitBox;

	UFUNCTION()
	void MakeBridge();

	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Distance")
	float ZDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName TagName;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
