// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestGroundCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MyCharacterMovementComponent.h"
#include "TestGroundGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "MySaveGame.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/DefaultValueHelper.h"
#include "MySaveGame.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATestGroundCharacter

ATestGroundCharacter::ATestGroundCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UMyCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	TGCMovementComponent = Cast<UMyCharacterMovementComponent>(GetCharacterMovement());
	if (TGCMovementComponent != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("movement component is loaded"));
	}
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

FCollisionQueryParams ATestGroundCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

void ATestGroundCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	bcanJump = true;

	if (IsValid(TGCMovementComponent) && TGCMovementComponent != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("movement component is available at begin play"));
	}
	else
	{
		TGCMovementComponent = Cast<UMyCharacterMovementComponent>(GetCharacterMovement()); //for some reason this became null even though we set it in the constructor. 
	}

	CurrentGameMode = Cast<ATestGroundGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	FTimerHandle FastTimer;
	GetWorld()->GetTimerManager().SetTimer(FastTimer, this, &ATestGroundCharacter::FastTick, 0.25f, true);

	//FTimerHandle SlowTimer;
	//GetWorld()->GetTimerManager().SetTimer(SlowTimer, this, &ATestGroundCharacter::SlowTick, 2.0f, true); 

}

FString ATestGroundCharacter::GetCellString()
{
	int x = GetActorLocation().X / 100;
	int y = GetActorLocation().Y / 100;
	int z = GetActorLocation().Z / 100;

	int vx = GetVelocity().X / 100;
	int vy = GetVelocity().Y / 100;
	int vz = GetVelocity().Z / 100;

	int p =  GetActorRotation().Pitch; 
	int yaw = GetActorRotation().Yaw; //only yaw really changes. 
	int r = GetActorRotation().Roll;

	int isWall = TGCMovementComponent->isWallRunning();

	return FString::Printf(TEXT("%d,%d,%d,"), x, y, z); //removed rotation for better visibility of squares
}

void ATestGroundCharacter::RememberCurrentState()
{
	FString key = GetCellString();
	UMySaveGame* value = GetStateAsSave();
	if (!StatesForCells.Contains(key))
	{
		StatesForCells.Add(key,TArray<UMySaveGame*>());
	}
	StatesForCells[key].Add(value);

}

void ATestGroundCharacter::FastTick()
{
	RememberCurrentState();
}

void ATestGroundCharacter::SlowTick()
{
	//pick a random cell, then a random state within the cell, and then restore it. 
	TArray<FString>KeyArray;
	StatesForCells.GetKeys(KeyArray);
	FString selectedCell = KeyArray[FMath::RandRange(0, StatesForCells.Num() - 1)];
	TArray<UMySaveGame*>StateArray = StatesForCells[selectedCell];
	UMySaveGame* selectedState = StateArray[FMath::RandRange(0, StateArray.Num() - 1)];
	RestoreStateFromSave(selectedState);

	//print cells statistics report
	UE_LOG(LogTemp, Warning, TEXT("///"));
	for (auto pair : StatesForCells)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s, %d"), *pair.Key, pair.Value.Num());
	}
}

void ATestGroundCharacter::SpawnDebugBoxForCell(FString cell)
{
	TArray<FString> StringComponents;

	// Parse the string into an array of individual components based on commas
	cell.ParseIntoArray(StringComponents, TEXT(","), true);

	int32 cx = FCString::Atoi(*StringComponents[0])* 100 ;
	int32 cy = FCString::Atoi(*StringComponents[1])* 100 ;
	int32 cz = FCString::Atoi(*StringComponents[2])* 100 + 50;

	DrawDebugBox(GetWorld(), FVector(cx,cy,cz), FVector(50.0f, 50.0f, 50.0f), FColor::Black, true, 3.0f, 0, 1.0f);
	
}

void ATestGroundCharacter::Tick(float DeltaSeconds)
{
	FString cell = GetCellString();
	if (!StatesForCells.Contains(cell))
	{
		RememberCurrentState();
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *cell);
		SpawnDebugBoxForCell(cell);
	}
	if (bCanGo)
	{
		RandomSeed();
	}
	//RandomMovement();
}

void ATestGroundCharacter::RandomSeed()
{
	RandSeed = UKismetMathLibrary::RandomInteger(5); //random number between 0-5
	bCanGo = false;
}

void ATestGroundCharacter::RandomMovement()
{
	switch (RandSeed)
	{
	case 0:
		AddMovementInput(FVector(0.0f, -1.0, 0.0f));
		break;
	case 1:
		AddMovementInput(FVector(0.0f, 1.0, 0.0f)); //
		break;
	case 2:
		AddMovementInput(FVector(1.0f, 0.0, 0.0f)); //
		break;
	case 3:
		AddMovementInput(FVector(-1.0f, 0.0, 0.0f)); //
		break;
	case 4:
		Jump();
		break;
	default:
		break;
	}
}

void ATestGroundCharacter::NewSeed()
{
	bCanSeed = true;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATestGroundCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATestGroundCharacter::Jump);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATestGroundCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATestGroundCharacter::Look);

		EnhancedInputComponent->BindAction(TestAction, ETriggerEvent::Triggered, this, &ATestGroundCharacter::TestFunction);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATestGroundCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATestGroundCharacter::StopSprinting);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATestGroundCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

	}
}

void ATestGroundCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATestGroundCharacter::Jump()
{
	Super::Jump();
	if (bcanJump)
	{
		this->LaunchCharacter(FVector(0, 0, 750), false, true);
		UE_LOG(LogTemp, Warning, TEXT("called super can jump"));
		bcanJump = false;
	}
}

void ATestGroundCharacter::Landed(const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("landed"));
	if (!bcanJump)
	{
		Super::StopJumping();
		bcanJump = true;
	}
}

void ATestGroundCharacter::Sprint()
{
	if (TGCMovementComponent)
	{
		TGCMovementComponent->SprintPressed();
		UE_LOG(LogTemp, Warning, TEXT("you are sprinting"));
	}

}

void ATestGroundCharacter::StopSprinting()
{
	UE_LOG(LogTemp, Warning, TEXT("you are not sprinting"));
	if (TGCMovementComponent)
	{
		TGCMovementComponent->SprintReleased();
	}
}

void ATestGroundCharacter::TestFunction()
{
	UE_LOG(LogTemp, Warning, TEXT("TEST FUNCTION IS WORKING"));
	const FInputActionValue test(FVector(1.0, 1.0f,1.0f));
	//Jump();
	
	bCanGo = true;

}


UMySaveGame* ATestGroundCharacter::GetStateAsSave()
{
	UMySaveGame* GameData = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
	GameData->Transform = GetTransform();
	GameData->PlayerLocation = GetActorLocation();
	GameData->CurrentMode = TGCMovementComponent->GetCustomMovementMode();
	GameData->Velocity = GetVelocity();

	return GameData;
}

void ATestGroundCharacter::RestoreStateFromSave(UMySaveGame* Save)
{
	SetActorLocation(Save->PlayerLocation);
	SetActorTransform(Save->Transform);
	TGCMovementComponent->SetMovementMode(MOVE_Custom, CMOVE_WallRun);
	TGCMovementComponent->Velocity = Save->Velocity;
}
