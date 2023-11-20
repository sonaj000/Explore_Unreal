// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GachaPon.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AGachaPon::AGachaPon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	SetRootComponent(MeshComponent);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	BoxComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AGachaPon::BeginPlay()
{
	Super::BeginPlay();
	FivestarR = 0.01f;
	FourstarR = 0.1f;
	ThreestarR = 0.79f;

	FeaturedR = 0.50f;
	
}

TArray<int> AGachaPon::Roll(int nRolls)
{
	TArray<int>round;
	for (int i{ 0 }; i < nRolls;i++)
	{
		float Rolls = UKismetMathLibrary::RandomFloat();
		if (Rolls < FivestarR)
		{
			float FR = UKismetMathLibrary::RandomFloat();
			if (FR < FeaturedR)
			{
				round.Add(6); //just a placeholder for now
			}
			else
			{
				round.Add(5);
			}
		}
		else if (Rolls < FourstarR)
		{
			round.Add(4);
		}
		else if (Rolls < ThreestarR)
		{
			round.Add(3);
		}
	}
	return round;
}



void AGachaPon::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AGachaPon::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

// Called every frame
void AGachaPon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

