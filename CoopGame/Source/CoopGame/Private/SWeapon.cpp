// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = m_meshComp;

	m_muzzleSocketName = "MuzzleFlashSocket";
	m_tracerTargetName = "BeamEnd";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASWeapon::Fire()
{
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

		FVector tracerEndPoint = traceEnd;

		FHitResult hit;
		if (GetWorld()->LineTraceSingleByChannel(hit, eyeLocation, traceEnd, ECC_Visibility, queryParams))
		{
			AActor* hitActor = hit.GetActor();
			UGameplayStatics::ApplyPointDamage(hitActor, 20.0f, shotDirection, hit, myOwner->GetInstigatorController(), this, m_damageType);

			if (m_impactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_impactEffect, hit.ImpactPoint, hit.ImpactNormal.Rotation(), FVector(0.33f));
			}

			tracerEndPoint = hit.ImpactPoint;
		}
		
		if (m_muzzleEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(m_muzzleEffect, m_meshComp, m_muzzleSocketName);
		}

		if (m_tracerEffect)
		{
			FVector muzzleLocation = m_meshComp->GetSocketLocation(m_muzzleSocketName);
			if (UParticleSystemComponent* tracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_tracerEffect, muzzleLocation))
			{
				tracerComp->SetVectorParameter(m_tracerTargetName, tracerEndPoint);
			}
		}

		//DrawDebugLine(GetWorld(), eyeLocation, traceEnd, FColor::White, false, 1.0f, 0, 1.0f);
	}
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
