// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	_defaultHealth = 100.0f;

	SetIsReplicated(true);
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		if (AActor* myOwner = GetOwner())
		{
			myOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}
	
	_health = _defaultHealth;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void USHealthComponent::HandleTakeAnyDamage(AActor* damagedActor, float damage, UDamageType const* damageType, AController* instigatedBy, AActor* damageCauser)
{
	if (damage <= 0.0f)
	{
		return;
	}

	_health = FMath::Clamp(_health - damage, 0.0f, _defaultHealth);

	OnHealthChanged.Broadcast(this, _health, damage, damageType, instigatedBy, damageCauser);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(_health));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, _health);
}
