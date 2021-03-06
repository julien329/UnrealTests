// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

///////////////////////////////////////////////////////////////////////////////////////////////////
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> _surfaceType;

	UPROPERTY()
	FVector_NetQuantize _traceTo;

	UPROPERTY()
	int8 _seed;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:

	virtual void BeginPlay() override;

	void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	void PlayFireEffects(FVector tracerEndPoint);

	void PlayImpactEffects(EPhysicalSurface surfaceType, FVector impactPoint);

	UFUNCTION()
	void OnRep_HitScanTrace();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* _meshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> _damageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName _muzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName _tracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* _muzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* _defaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* _fleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* _tracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> _fireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float _baseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float _vulnerableDamageMulti;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float _rateOfFire;

	float _timeBetweenShots;

	FTimerHandle _timerHandle_timeBetweenShots;

	float _lastFireTime;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace _hitScanTrace;

public:

	void StartFire();
	void StopFire();
};