// Fill out your copyright notice in the Description page of Project Settings.


#include "QueueMachine.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/TestWidget.h"
#include "TestGround/TestGroundCharacter.h"

// Sets default values
AQueueMachine::AQueueMachine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	SetRootComponent(MeshComponent);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AQueueMachine::OnComponentBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AQueueMachine::OnOverlapEnd);


}

// Called when the game starts or when spawned
void AQueueMachine::BeginPlay()
{
	Super::BeginPlay();

	Widget = CreateWidget<UTestWidget>(GetWorld(), TestClass);
	if (Widget != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("added"))
	}
	bCanCollide = true;
	
}

void AQueueMachine::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != this && OtherActor->IsA(APawn::StaticClass()) && bCanCollide)
	{
		UE_LOG(LogTemp, Warning, TEXT("on compoennt overlap begin"));
		if (Widget != nullptr)
		{
			Widget->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("added to the viewport"));
			ACharacter* hold = Cast<ACharacter>(OtherActor);
			APlayerController* CurrP = Cast<APlayerController>(hold->GetController());
			if (CurrP)
			{
				CurrP->bShowMouseCursor = true;
			}

		}
		bCanCollide = false;
	}
}

void AQueueMachine::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != this && OtherActor->IsA(APawn::StaticClass()) && !bCanCollide)
	{
		//spawn umg 
		if (Widget != nullptr)
		{
			Widget->RemoveFromViewport();
			UE_LOG(LogTemp, Warning, TEXT("on compoennt overlap end"));
			ACharacter* hold = Cast<ACharacter>(OtherActor);
			APlayerController* CurrP = Cast<APlayerController>(hold->GetController());
			if (CurrP)
			{
				CurrP->bShowMouseCursor = false;
			}
		}
		bCanCollide = true;
	}
}

// Called every frame
void AQueueMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

