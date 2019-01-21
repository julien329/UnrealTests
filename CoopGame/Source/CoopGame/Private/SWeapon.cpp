// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

static int32 _debugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), _debugWeaponDrawing, TEXT("Draw debug lines for weapons."), ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	_meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = _meshComp;

	_muzzleSocketName = "MuzzleFlashSocket";
	_tracerTargetName = "BeamEnd";

	_baseDamage = 20.0f;
	_vulnerableDamageMulti = 4.0f;

	_rateOfFire = 600;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	_timeBetweenShots = 60.0f / _rateOfFire;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	if (AActor* myOwner = GetOwner())
	{
		FVector eyeLocation;
		FRotator eyeRotation;
		myOwner->GetActorEyesViewPoint(eyeLocation, eyeRotation);

		FVector shotDirection = eyeRotation.Vector();
		FVector traceEnd = eyeLocation + (shotDirection * 10000);

		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(myOwner);
		queryParams.AddIgnoredActor(this);
		queryParams.bTraceComplex = true;
		queryParams.bReturnPhysicalMaterial = true;

		FVector tracerEndPoint = traceEnd;

		EPhysicalSurface surfaceType = EPhysicalSurface::SurfaceType_Default;

		FHitResult hit;
		if (GetWorld()->LineTraceSingleByChannel(hit, eyeLocation, traceEnd, COLLISION_WEAPON, queryParams))
		{
			AActor* hitActor = hit.GetActor();

			surfaceType = UPhysicalMaterial::DetermineSurfaceType(hit.PhysMaterial.Get());

			float actualDamage = _baseDamage;
			if (surfaceType == SURFACE_FLESHVULNERABLE)
			{
				actualDamage *= _vulnerableDamageMulti;
			}

			UGameplayStatics::ApplyPointDamage(hitActor, actualDamage, shotDirection, hit, myOwner->GetInstigatorController(), this, _damageType);

			PlayImpactEffects(surfaceType, hit.ImpactPoint);

			tracerEndPoint = hit.ImpactPoint;

			_hitScanTrace._surfaceType = surfaceType;
		}

		PlayFireEffects(tracerEndPoint);

		if (Role == ROLE_Authority)
		{
			_hitScanTrace._traceTo = tracerEndPoint;
			_hitScanTrace._surfaceType = surfaceType;
			_hitScanTrace._seed = (_hitScanTrace._seed + 1) % 127;
		}

		_lastFireTime = GetWorld()->TimeSeconds;

		if (_debugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), eyeLocation, traceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ASWeapon::ServerFire_Validate()
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::PlayFireEffects(FVector tracerEndPoint)
{
	if (_muzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(_muzzleEffect, _meshComp, _muzzleSocketName);
	}

	if (_tracerEffect)
	{
		FVector muzzleLocation = _meshComp->GetSocketLocation(_muzzleSocketName);
		if (UParticleSystemComponent* tracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _tracerEffect, muzzleLocation))
		{
			tracerComp->SetVectorParameter(_tracerTargetName, tracerEndPoint);
		}
	}

	if (APawn* myOwner = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* playerController = Cast<APlayerController>(myOwner->GetController()))
		{
			playerController->ClientPlayCameraShake(_fireCamShake);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::PlayImpactEffects(EPhysicalSurface surfaceType, FVector impactPoint)
{
	UParticleSystem* selectedEffect = nullptr;
	switch (surfaceType)
	{
		case SURFACE_FLESHDEFAULT:
		case SURFACE_FLESHVULNERABLE:
			selectedEffect = _fleshImpactEffect;
			break;
		default:
			selectedEffect = _defaultImpactEffect;
			break;
	}

	if (selectedEffect)
	{
		FVector muzzleLocation = _meshComp->GetSocketLocation(_muzzleSocketName);

		FVector shotDirection = impactPoint - muzzleLocation;
		shotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), selectedEffect, impactPoint, shotDirection.Rotation(), FVector(0.33f));
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(_hitScanTrace._traceTo);
	PlayImpactEffects(_hitScanTrace._surfaceType, _hitScanTrace._traceTo);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::StartFire()
{
	float firstDelay = FMath::Max(_timeBetweenShots - (GetWorld()->TimeSeconds - _lastFireTime), 0.0f);
	GetWorldTimerManager().SetTimer(_timerHandle_timeBetweenShots, this, &ASWeapon::Fire, _timeBetweenShots, true, firstDelay);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(_timerHandle_timeBetweenShots);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, _hitScanTrace, COND_SkipOwner);
}
