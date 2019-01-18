// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	_springArmComp->bUsePawnControlRotation = true;
	_springArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	_cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	_cameraComp->SetupAttachment(_springArmComp);

	_zoomedFOV = 65.0f;
	_zoomInterpSpeed = 20.0;

	_weaponAttachSocketName = "WeaponSocket";
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	_defaultFOV = _cameraComp->FieldOfView;

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	_currentWeapon = GetWorld()->SpawnActor<ASWeapon>(_starterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, spawnParams);
	if (_currentWeapon) 
	{
		_currentWeapon->SetOwner(this);
		_currentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, _weaponAttachSocketName);
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
