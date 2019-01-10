// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGameState.h"
#include "Engine/World.h"
#include "FPSPlayerController.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
void AFPSGameState::MulticastOnMissionComplete_Implementation(APawn* instigatorPawn, bool missionSuccess)
{
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; it++) 
	{
		AFPSPlayerController* playerController = Cast<AFPSPlayerController>(it->Get());
		if (playerController && playerController->IsLocalController())
		{
			playerController->OnMissionComplete(instigatorPawn, missionSuccess);

			if (APawn* pawn = playerController->GetPawn())
			{
				pawn->DisableInput(playerController);
			}
		}
	}
}
