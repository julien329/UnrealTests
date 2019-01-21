// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoopGame.h"
#include "Engine/World.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SCharacter.h"
#include "SHealthComponent.h"
#include "SWeapon.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	_springArmComp->bUsePawnControlRotation = true;
	_springArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	_healthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	_cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	_cameraComp->SetupAttachment(_springArmComp);

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	_zoomedFOV = 65.0f;
	_zoomInterpSpeed = 20.0;

	_weaponAttachSocketName = "WeaponSocket";
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	_defaultFOV = _cameraComp->FieldOfView;
	_healthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	if (Role == ROLE_Authority)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		_currentWeapon = GetWorld()->SpawnActor<ASWeapon>(_starterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, spawnParams);
		if (_currentWeapon)
		{
			_currentWeapon->SetOwner(this);
			_currentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, _weaponAttachSocketName);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::MoveForward(float value)
{
	AddMovementInput(GetActorForwardVector() * value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector() * value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::BeginCrouch()
{
	Crouch();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::EndCrouch()
{
	UnCrouch();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::BeginZoom()
{
	_wantsToZoom = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::EndZoom()
{
	_wantsToZoom = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::StartFire()
{
	if (_currentWeapon)
	{
		_currentWeapon->StartFire();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::StopFire()
{
	if (_currentWeapon)
	{
		_currentWeapon->StopFire();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::OnHealthChanged(USHealthComponent* healthComp, float health, float healthDelta, UDamageType const* damageType, AController* instigatedBy, AActor* damageCauser)
{
	if (health <= 0.0f && !_isDead)
	{
		_isDead = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float targetFOV = _wantsToZoom ? _zoomedFOV : _defaultFOV;

	float newFOV = FMath::FInterpTo(_cameraComp->FieldOfView, targetFOV, DeltaTime, _zoomInterpSpeed);
	_cameraComp->SetFieldOfView(newFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Zoom", EInputEvent::IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", EInputEvent::IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &ASCharacter::StopFire);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
FVector ASCharacter::GetPawnViewLocation() const
{
	if (_cameraComp)
	{
		return _cameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, _currentWeapon);
	DOREPLIFETIME(ASCharacter, _isDead);
}
