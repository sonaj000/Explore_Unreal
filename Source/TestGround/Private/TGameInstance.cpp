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

void UTGameInstance::Host()
{
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	try
	{
		World->ServerTravel("/Game/Levels/FirstMap"); // The server will call APlayerController::ClientTravel for all client players that are connected.
	}
	catch (const std::exception&)
	{
		UE_LOG(LogTemp, Warning, TEXT("no travel"));
	}
}

void UTGameInstance::Join(const FString& Address)
{
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;
	UE_LOG(LogTemp, Warning, TEXT("trying joing"));
	PlayerController->ClientTravel("/Game/Levels/FirstMap", ETravelType::TRAVEL_Absolute);
}
