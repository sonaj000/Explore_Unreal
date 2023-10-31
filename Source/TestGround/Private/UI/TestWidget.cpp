// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TestWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "TGameInstance.h"

void UTestWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HostB)
	{
		HostB->OnClicked.AddUniqueDynamic(this, &UTestWidget::UTestWidget::HostClick); 
	}
	if (JoinB)
	{
		JoinB->OnClicked.AddUniqueDynamic(this, &UTestWidget::JoinClick);
	}

}

void UTestWidget::HostClick()
{
	UTGameInstance* Holder = Cast<UTGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	Holder->Host(); 
}

void UTestWidget::JoinClick()
{
	UTGameInstance* Holder = Cast<UTGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	UE_LOG(LogTemp, Warning, TEXT("join clicking"));
	Holder->Join("/Game/Levels/FirstMap");
}

