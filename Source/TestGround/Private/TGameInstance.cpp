// Fill out your copyright notice in the Description page of Project Settings.


#include "TGameInstance.h"

UTGameInstance::UTGameInstance(const FObjectInitializer& ObjectInitializer)
{
	UE_LOG(LogTemp, Warning, TEXT("game constructor"));
}

void UTGameInstance::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("init for game instance"));
}

