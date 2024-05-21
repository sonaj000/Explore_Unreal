// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestWidget.generated.h"

/**
 * 
 */

class UTextBlock;
class UButton;
UCLASS()
class TESTGROUND_API UTestWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* HostB;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* JoinB;

};
