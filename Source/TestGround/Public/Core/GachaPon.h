// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GachaPon.generated.h"

class UBoxComponent;

UCLASS()
class TESTGROUND_API AGachaPon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGachaPon(); //base class for gachapon. for the different types of pity, we will have them inherit from this base class. 

	UPROPERTY(BlueprintReadWrite)
	TArray<int>GachaRoll;

	UPROPERTY(VisibleAnywhere)
	int Counter; //counter for the gachapon machine. 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual TArray<int> Roll(int nRolls);

	float FivestarR;
	float FourstarR;
	float ThreestarR;

	float FeaturedR;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	UBoxComponent* BoxComponent;

	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
