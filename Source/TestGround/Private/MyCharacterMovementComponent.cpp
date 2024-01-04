// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TestGround/TestGroundCharacter.h"
#include "DrawDebugHelpers.h"

UMyCharacterMovementComponent::UMyCharacterMovementComponent()
{
    bWantsToSprint = false;

    Walk_MaxWalkSpeed = 600;
    Sprint_MaxWalkSpeed = 1500;
}

void UMyCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
    Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

    if (MovementMode == MOVE_Walking)
    {
        //UE_LOG(LogTemp, Warning, TEXT("we are in move walking mode"));
        if (bWantsToSprint)
        {
            MaxWalkSpeed = Sprint_MaxWalkSpeed;
            //UE_LOG(LogTemp, Warning, TEXT("we are sprinting : %f"), MaxWalkSpeed);
        }
        else
        {
            MaxWalkSpeed = Walk_MaxWalkSpeed;
            //UE_LOG(LogTemp, Warning, TEXT("we are walking : %f"),MaxWalkSpeed);
        }
    }
}

bool UMyCharacterMovementComponent::IsMovingOnGround() const
{
    return Super::IsMovingOnGround();
}

bool UMyCharacterMovementComponent::CanCrouchInCurrentState() const
{
    return false;
}

float UMyCharacterMovementComponent::GetMaxSpeed() const
{
    if (IsMovementMode(MOVE_Walking) && bWantsToSprint) return MaxSprintSpeed;

    if (MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

    switch (CustomMovementMode)
    {
        case CMOVE_WallRun:
            return MaxWallRunSpeed;
        default:
            UE_LOG(LogTemp, Warning, TEXT("invalid movement mode"));
            return -1.0f;
    }
}

float UMyCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
    if (MovementMode != MOVE_Custom)
    {
        return Super::GetMaxBrakingDeceleration();
    }
    switch (CustomMovementMode)
    {
    case CMOVE_WallRun:
        return 0.0f;
    default:
        UE_LOG(LogTemp, Warning, TEXT("invalid movement mode"));
        return -1.0f;
    }
    
}

bool UMyCharacterMovementComponent::CanAttemptJump() const
{
    UE_LOG(LogTemp, Warning, TEXT("Can attempt jump"));
    return Super::CanAttemptJump() || isWallRunning();
}

bool UMyCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
    UE_LOG(LogTemp, Warning, TEXT("do jump"));
    bool bWasWallRunning = isWallRunning();
    if (Super::DoJump(bReplayingMoves)) //check to see if we are doing the jump
    {
        if (bWasWallRunning)
        {
            FVector Start = UpdatedComponent->GetComponentLocation();
            FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
            FVector End = Safe_bWallRunRight ? Start + CastDelta : Start - CastDelta; //cast ray to either left or right 
            auto Params = TestGroundOwner->GetIgnoreCharacterParams();
            FHitResult WallHit;
            GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "Vehicle", Params);
            Velocity += WallHit.Normal * WallJumpOffForce;
        }
        return true;
    }
    return false;
}

bool UMyCharacterMovementComponent::TryWallRun()
{
    if (!IsFalling())
    {
       UE_LOG(LogTemp, Warning, TEXT("isnotfalling"));
        return false;
    }
    if (Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2)) //if horizontal velocity is less than minwallspeed. minwallspeed evlautes to 40,000. 
    {
        float hold = Velocity.SizeSquared2D();
        UE_LOG(LogTemp, Warning, TEXT("velocity issue and velocity is : %f"), hold);
        return false;
    }
    //if (Velocity.Z < -MaxVerticalWallRunSpeed) //check z velocity so we could comment this out if don't want to automatically wallrun after falling fast. 
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("velocity z issue : %f"),Velocity.Z);
    //    return false;
    //}

    FVector Start = UpdatedComponent->GetComponentLocation();
    FVector LeftEnd = Start - UpdatedComponent->GetRightVector() * CapR() * 2.0;
    FVector RightEnd = Start + UpdatedComponent->GetRightVector() * CapR() * 2.0;
    auto Params = TestGroundOwner->GetIgnoreCharacterParams();
    FHitResult FloorHit, WallHit;
    // Check Player Height
    DrawDebugLine(GetWorld(), Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight), FColor::Black, false, 2.0f, 0, 2.0f);
    if (GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight), "Vehicle", Params)) //profile name is a collision type
    {
        AActor* hit = FloorHit.GetActor();
        //UE_LOG(LogTemp, Warning, TEXT("player height issue : %s"),*hit->GetName());
        return false;
    }

    // Left Cast
    DrawDebugLine(GetWorld(), Start, LeftEnd, FColor::Red, false, 2.0f, 0, 2.0f);
    GetWorld()->LineTraceSingleByProfile(WallHit, Start, LeftEnd, "Vehicle", Params);
    //UE_LOG(LogTemp, Warning, TEXT("velocity is : %f"), Velocity);
    //UE_LOG(LogTemp, Warning, TEXT("wallhit normal is : %f"), WallHit.Normal);
    //UE_LOG(LogTemp, Warning, TEXT("left wall has a valid blocking hit : %s"), (WallHit.IsValidBlockingHit() ? TEXT("true") : TEXT("false")));
    if (WallHit.IsValidBlockingHit() && (Velocity | WallHit.Normal) < 0)
    {
        Safe_bWallRunRight = false;
    }
    // Right Cast
    else
    {
        DrawDebugLine(GetWorld(), Start, RightEnd, FColor::Blue, false, 2.0f, 0, 2.0f);
        GetWorld()->LineTraceSingleByProfile(WallHit, Start, RightEnd, "Vehicle", Params);
        //UE_LOG(LogTemp, Warning, TEXT("right wall has a valid blocking hit : %s"), (WallHit.IsValidBlockingHit() ? TEXT("true") : TEXT("false")));
        if (WallHit.IsValidBlockingHit() && (Velocity | WallHit.Normal) < 0)
        {
            Safe_bWallRunRight = true;
        }
        else
        {
            //UE_LOG(LogTemp, Warning, TEXT("right cast is wrong"));
            return false;
        }
    }
    FVector ProjectedVelocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
    if (ProjectedVelocity.SizeSquared2D() < pow(MinWallRunSpeed, 2))
    {
        //UE_LOG(LogTemp, Warning, TEXT("projected velocity issue"));
        return false;
    }
    // Passed all conditions
    Velocity = ProjectedVelocity;
    Velocity.Z = FMath::Clamp(Velocity.Z, 0.f, MaxVerticalWallRunSpeed);
    SetMovementMode(MOVE_Custom, CMOVE_WallRun);
    UE_LOG(LogTemp, Warning, TEXT("Starting wall run"));
    return true;
}

void UMyCharacterMovementComponent::PhysWallRun(float deltaTime, int32 Iterations)
{
    if (deltaTime < MIN_TICK_TIME)
    {
        return;
    }
    if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
    {
        Acceleration = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        return;
    }

    bJustTeleported = false;
    float remainingTime = deltaTime;
    // Perform the move
    while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
    {
        Iterations++;
        bJustTeleported = false;
        const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
        remainingTime -= timeTick;
        const FVector OldLocation = UpdatedComponent->GetComponentLocation();

        FVector Start = UpdatedComponent->GetComponentLocation();
        FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
        FVector End = Safe_bWallRunRight ? Start + CastDelta : Start - CastDelta;
        auto Params = TestGroundOwner->GetIgnoreCharacterParams();
        float SinPullAwayAngle = FMath::Sin(FMath::DegreesToRadians(WallRunPullAwayAngle));
        FHitResult WallHit;
        GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "Vehicle", Params);
        bool bWantsToPullAway = WallHit.IsValidBlockingHit() && !Acceleration.IsNearlyZero() && (Acceleration.GetSafeNormal() | WallHit.Normal) > SinPullAwayAngle;
        if (!WallHit.IsValidBlockingHit() || bWantsToPullAway)
        {
            SetMovementMode(MOVE_Falling);
            StartNewPhysics(remainingTime, Iterations);
            return;
        }
        // Clamp Acceleration
        Acceleration = FVector::VectorPlaneProject(Acceleration, WallHit.Normal);
        Acceleration.Z = 0.f;
        // Apply acceleration
        CalcVelocity(timeTick, 0.f, false, GetMaxBrakingDeceleration());
        Velocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
        float TangentAccel = Acceleration.GetSafeNormal() | Velocity.GetSafeNormal2D();
        bool bVelUp = Velocity.Z > 0.f;
        Velocity.Z += GetGravityZ() * WallRunGravityScaleCurve->GetFloatValue(bVelUp ? 0.f : TangentAccel) * timeTick;
        if (Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2) || Velocity.Z < -MaxVerticalWallRunSpeed)
        {
            SetMovementMode(MOVE_Falling);
            StartNewPhysics(remainingTime, Iterations);
            return;
        }

        // Compute move parameters
        const FVector Delta = timeTick * Velocity; // dx = v * dt
        const bool bZeroDelta = Delta.IsNearlyZero();
        if (bZeroDelta)
        {
            remainingTime = 0.f;
        }
        else
        {
            FHitResult Hit;
            SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
            FVector WallAttractionDelta = -WallHit.Normal * WallAttractionForce * timeTick;
            SafeMoveUpdatedComponent(WallAttractionDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
        }
        if (UpdatedComponent->GetComponentLocation() == OldLocation)
        {
            remainingTime = 0.f;
            break;
        }
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick; // v = dx / dt
    }


    FVector Start = UpdatedComponent->GetComponentLocation();
    FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
    FVector End = Safe_bWallRunRight ? Start + CastDelta : Start - CastDelta;
    auto Params = TestGroundOwner->GetIgnoreCharacterParams();
    FHitResult FloorHit, WallHit;
    GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "Vehicle", Params);
    GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight * .5f), "Vehicle", Params);
    if (FloorHit.IsValidBlockingHit() || !WallHit.IsValidBlockingHit() || Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2))
    {
        SetMovementMode(MOVE_Falling);
    }
}

void UMyCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    Super::PhysCustom(deltaTime, Iterations);

    switch (CustomMovementMode)
    {
    case CMOVE_WallRun:
        PhysWallRun(deltaTime, Iterations);
        break;
    default:
        UE_LOG(LogTemp, Fatal, TEXT("invalid movement mode"));
 
    }
}

void UMyCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

    if (IsFalling())
    {
        bOrientRotationToMovement = true;
    }

    if (isWallRunning())
    {
        FVector Start = UpdatedComponent->GetComponentLocation();
        FVector End = Start + UpdatedComponent->GetRightVector() * CapR() * 2;
        auto Params = TestGroundOwner->GetIgnoreCharacterParams();
        FHitResult WallHit;
        Safe_bWallRunRight = GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "Vehicle", Params);
    }
}

float UMyCharacterMovementComponent::CapR() const
{
    return GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UMyCharacterMovementComponent::CapHH() const
{
    return GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

bool UMyCharacterMovementComponent::IsMovementMode(EMovementMode inMovementMode) const
{
    return inMovementMode == MovementMode;
}

bool UMyCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
    return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

void UMyCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
    //UE_LOG(LogTemp, Warning, TEXT("before movement"));
    if (IsFalling())
    {
        TryWallRun();
        UE_LOG(LogTemp, Warning, TEXT("value of try wall run is : %s"), (TryWallRun() ? TEXT("true") : TEXT("false")));
    }

    Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UMyCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
}

uint8 UMyCharacterMovementComponent::GetCustomMovementMode() const
{
    return CustomMovementMode;
}

void UMyCharacterMovementComponent::SprintPressed()
{

    bWantsToSprint = true;
    UE_LOG(LogTemp, Warning, TEXT("bwants to spring is valid"));

}

void UMyCharacterMovementComponent::SprintReleased()
{
    bWantsToSprint = false;
}
