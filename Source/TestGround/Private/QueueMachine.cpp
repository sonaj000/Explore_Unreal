// Fill out your copyright notice in the Description page of Project Settings.


#include "QueueMachine.h"
#include "Components/BoxComponent.h"

// Sets default values
AQueueMachine::AQueueMachine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	SetRootComponent(MeshComponent);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	BoxComponent->SetupAttachment(RootComponent);


}

// Called when the game starts or when spawned
void AQueueMachine::BeginPlay()
{
	Super::BeginPlay();
	
}

void AQueueMachine::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

// Called every frame
void AQueueMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

