// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSExtractionZone.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFPSExtractionZone::AFPSExtractionZone()
{
	m_overlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
	m_overlapComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	m_overlapComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	m_overlapComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	m_overlapComp->SetBoxExtent(FVector(200.f));
	RootComponent = m_overlapComp;

	m_overlapComp->SetHiddenInGame(false);

	m_overlapComp->OnComponentBeginOverlap.AddDynamic(this, &AFPSExtractionZone::HandleOverlap);

	m_decalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	m_decalComp->DecalSize = FVector(200.0f);
	m_decalComp->SetupAttachment(RootComponent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AFPSExtractionZone::HandleOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool fromSweep, FHitResult const& sweepResult)
{
	AFPSCharacter* myPawn = Cast<AFPSCharacter>(otherActor);
	if (!myPawn)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Overlapped with extraction zone!"));

	if (myPawn->IsCarryingObjective)
	{
		if (AFPSGameMode* gameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
		{
			gameMode->CompleteMission(myPawn, true);
		}
	}
	else
	{
		UGameplayStatics::PlaySound2D(this, m_objectiveMissingSound);
	}
}