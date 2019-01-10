// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSAIGuard.generated.h"

class UPawnSensingComponent;

UENUM(BlueprintType)
enum class EAIState : uint8 
{
	Idle,
	Suspicious,
	Alerted
};

UCLASS()
class FPSGAME_API AFPSAIGuard : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSAIGuard();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	UPawnSensingComponent* m_pawnSensingComp;

	UFUNCTION()
	void OnPawnSeen(APawn* seenPawn);

	UFUNCTION()
	void OnNoiseHeard(APawn* noiseInstigator, FVector const& location, float volume);

	FRotator m_originalRotation;
	FTimerHandle m_timerHandle_resetOrientation;

	UFUNCTION()
	void ResetOrientation();

	EAIState m_guardState;

	void SetGuardState(EAIState newState);

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void OnStateChanged(EAIState newState);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
