// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSExtractionZone.generated.h"

class UBoxComponent;
class UDecalComponent;

UCLASS()
class FPSGAME_API AFPSExtractionZone : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFPSExtractionZone();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* m_overlapComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* m_decalComp;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool fromSweep, FHitResult const& sweepResult);

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* m_objectiveMissingSound;
};
