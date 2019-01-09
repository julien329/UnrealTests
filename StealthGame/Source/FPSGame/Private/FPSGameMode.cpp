// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "FPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AFPSGameMode::AFPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_Player"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSHUD::StaticClass();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AFPSGameMode::CompleteMission(APawn* instigatorPawn)
{
	if (instigatorPawn)
	{
		instigatorPawn->DisableInput(nullptr);

		if (SpectatingViewpointClass)
		{
			TArray<AActor*> returnedActors;
			UGameplayStatics::GetAllActorsOfClass(this, SpectatingViewpointClass, returnedActors);

			if (returnedActors.Num() > 0)
			{
				AActor* newViewTarget = returnedActors[0];

				if (APlayerController* playerController = Cast<APlayerController>(instigatorPawn->GetController()))
				{
					playerController->SetViewTargetWithBlend(newViewTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
				}
			}
		}
		else 
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatingViewpointClass is nullptr. Cannot change spectating view target"));
		}
	}

	OnMissionCompleted(instigatorPawn);
}
