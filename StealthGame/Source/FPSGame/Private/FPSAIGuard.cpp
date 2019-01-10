// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "FPSGameMode.h"
#include "Engine/World.h"


// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_pawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	m_pawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	m_pawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnNoiseHeard);

	m_guardState = EAIState::Idle;
}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();

	m_originalRotation = GetActorRotation();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AFPSAIGuard::OnPawnSeen(APawn* seenPawn)
{
	DrawDebugSphere(GetWorld(), seenPawn->GetActorLocation(), 32.0f, 12, FColor::Red, false, 10.0f);

	if (!seenPawn)
	{
		return;
	}

	if (AFPSGameMode* gameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		gameMode->CompleteMission(seenPawn, false);
	}

	SetGuardState(EAIState::Alerted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AFPSAIGuard::OnNoiseHeard(APawn* noiseInstigator, FVector const& location, float volume)
{
	if (m_guardState == EAIState::Alerted)
	{
		return;
	}

	DrawDebugSphere(GetWorld(), location, 32.0f, 12, FColor::Green, false, 10.0f);

	FVector direction = location - GetActorLocation();
	direction.Normalize();

	FRotator newLookAt = FRotationMatrix::MakeFromX(direction).Rotator();
	newLookAt.Pitch = 0.0f;
	newLookAt.Roll = 0.0f;

	SetActorRotation(newLookAt);

	GetWorldTimerManager().ClearTimer(m_timerHandle_resetOrientation);
	GetWorldTimerManager().SetTimer(m_timerHandle_resetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);

	SetGuardState(EAIState::Suspicious);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AFPSAIGuard::ResetOrientation()
{
	if (m_guardState == EAIState::Alerted)
	{
		return;
	}

	SetActorRotation(m_originalRotation);
	SetGuardState(EAIState::Idle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AFPSAIGuard::SetGuardState(EAIState newState)
{
	if (m_guardState == newState)
	{
		return;
	}

	m_guardState = newState;

	OnStateChanged(m_guardState);
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
