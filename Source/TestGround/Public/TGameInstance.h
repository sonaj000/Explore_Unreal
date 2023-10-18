// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TESTGROUND_API UTGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UTGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init();

	UFUNCTION(Exec)
	void Host();

	UFUNCTION(Exec)
	void Join(const FString& Address);


	
};
