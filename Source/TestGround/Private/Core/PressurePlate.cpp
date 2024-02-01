// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PressurePlate.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
APressurePlate::APressurePlate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Tile = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh Component Tile"));
	SetRootComponent(Tile);

	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("hit box"));
	HitBox->SetupAttachment(RootComponent);

	HitBox->OnComponentBeginOverlap.AddDynamic(this, &APressurePlate::OnComponentBeginOverlap);


	ZDistance = 500;
	TagName = "Bridge0";

}

void APressurePlate::MakeBridge()
{
}

void APressurePlate::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()) && !bIsPressed)
	{
		bIsPressed = true;
		UE_LOG(LogTemp, Warning, TEXT("pressed the PRESSURE PLATE"));
	}
}

// Called when the game starts or when spawned
void APressurePlate::BeginPlay()
{
	Super::BeginPlay();

	bIsPressed = false;

	if (Bridge == nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		Bridge = GetWorld()->SpawnActor<AActor>(BridgeActor, this->GetActorLocation() + GetActorForwardVector() * ZDistance, FRotator::ZeroRotator, SpawnParams);
		Bridge->SetActorHiddenInGame(true);
		Bridge->SetActorEnableCollision(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("there is no valid bridge pointer"));
	}
}

// Called every frame
void APressurePlate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("bisPressed is now : %s"), (bIsPressed ? TEXT("true") : TEXT("false")));
	Bridge->SetActorHiddenInGame(!bIsPressed);
	Bridge->SetActorEnableCollision(bIsPressed);

}

