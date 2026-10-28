// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "CXPlayerState.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "CXMine.h"
#include "CXChatGameState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	MineClass = ACXMine::StaticClass();
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);
		if (AttackAction) EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACXPawn::Attack);
		else UE_LOG(LogTemplateVehicle, Warning, TEXT("%s: AttackAction is not assigned in vehicle Blueprint defaults."), *GetName());
		if (MineAction) EnhancedInputComponent->BindAction(MineAction, ETriggerEvent::Started, this, &ACXPawn::LayMine);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	UpdateMatchInputLock();
	Super::Tick(Delta);
	if (bThrottleBroken) ChaosVehicleMovement->SetThrottleInput(0.f);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	if (!IsMatchPlaying()) { UpdateMatchInputLock(); return; }
	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	if (!IsMatchPlaying()) { UpdateMatchInputLock(); return; }
	// get the input magnitude for the throttle
	float ThrottleValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetThrottleInput(bThrottleBroken ? 0.f : ThrottleValue);
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void ACXPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
	if (!IsMatchPlaying()) { UpdateMatchInputLock(); return; }
	bHandbrakeHeld = true;
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
	bHandbrakeHeld = false;
	if (!IsMatchPlaying()) { UpdateMatchInputLock(); return; }
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
	if (!IsMatchPlaying()) return;
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void ACXPawn::Attack(const FInputActionValue& Value)
{
	if (!IsMatchPlaying()) return;
	ServerAttack();
}

void ACXPawn::ServerAttack_Implementation()
{
	if (!IsMatchPlaying()) return;
	const FVector Start = GetActorLocation() + GetActorForwardVector() * 180.0f + FVector(0, 0, 50.0f);
	const FVector End = Start + GetActorForwardVector() * AttackRange;
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VehicleAttack), false, this);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	MulticastDrawAttackTrace(Start, bHit ? Hit.ImpactPoint : End, bHit);
	if (ACXPawn* Target = bHit ? Cast<ACXPawn>(Hit.GetActor()) : nullptr)
	{
		UGameplayStatics::ApplyPointDamage(Target, 5.0f, GetActorForwardVector(), Hit, GetController(), this, UDamageType::StaticClass());
		MulticastPlayAttackHitEffect(Hit.ImpactPoint);
	}
}

float ACXPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || DamageAmount <= 0.0f) return 0.0f;
	ACXPawn* Attacker = Cast<ACXPawn>(DamageCauser);
	ACXPlayerState* VictimState = GetController() ? GetController()->GetPlayerState<ACXPlayerState>() : nullptr;
	ACXPlayerState* AttackerState = Attacker && Attacker->GetController() ? Attacker->GetController()->GetPlayerState<ACXPlayerState>() : nullptr;
	if (Attacker && Attacker != this && VictimState && DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FName Bone = static_cast<const FPointDamageEvent&>(DamageEvent).HitInfo.BoneName;
		for (const FChaosWheelSetup& Wheel : ChaosVehicleMovement->WheelSetups)
		{
			if (Bone != NAME_None && (Bone == Wheel.BoneName || GetMesh()->BoneIsChildOf(Bone, Wheel.BoneName)))
			{
				VictimState->ApplyWheelPenalty(10.f);
				break;
			}
		}
	}
	if (!AttackerState || !VictimState || VictimState->GetCoins() < 5) return 0.0f;
	if (VictimState->SpendCoins(5)) AttackerState->AddCoins(5);
	return DamageAmount;
}

void ACXPawn::MulticastPlayAttackHitEffect_Implementation(FVector Location)
{
	if (AttackHitParticle) UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitParticle, Location);
}

#undef LOCTEXT_NAMESPACE
void ACXPawn::LayMine(const FInputActionValue&) { if (IsMatchPlaying()) ServerLayMine(); }

void ACXPawn::ServerLayMine_Implementation()
{
	if (!IsMatchPlaying()) return;
	if (!GetController() || !MineClass || GetWorld()->GetTimeSeconds() < NextMineTime) return;
	NextMineTime = GetWorld()->GetTimeSeconds() + 1.f;
	FVector Location = GetActorLocation() - GetActorForwardVector() * 350.f;
	FHitResult Ground;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(MineGround),false,this);
	if (GetWorld()->LineTraceSingleByChannel(Ground,Location+FVector(0,0,100),Location-FVector(0,0,500),ECC_Visibility,Query)) Location=Ground.ImpactPoint+FVector(0,0,15);
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<ACXMine>(MineClass,Location,GetActorRotation(),Params);
}

void ACXPawn::DisableThrottleForMine()
{
	if (!HasAuthority()) return;
	bThrottleBroken = true;
	OnRep_ThrottleBroken();
	ClientSetThrottleBroken(true);
	ForceNetUpdate();
	GetWorldTimerManager().SetTimer(ThrottleRepairTimer,this,&ThisClass::RestoreThrottle,2.f,false);
}
void ACXPawn::RestoreThrottle()
{
	bThrottleBroken = false;
	ClientSetThrottleBroken(false);
	ForceNetUpdate();
}
void ACXPawn::ClientSetThrottleBroken_Implementation(bool bBroken)
{
	bThrottleBroken = bBroken;
	OnRep_ThrottleBroken();
}
void ACXPawn::OnRep_ThrottleBroken()
{
	if (bThrottleBroken && ChaosVehicleMovement) ChaosVehicleMovement->SetThrottleInput(0.f);
}
void ACXPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACXPawn,bThrottleBroken);
}
void ACXPawn::MulticastDrawAttackTrace_Implementation(FVector Start, FVector End, bool bHit)
{
	if (GetNetMode() == NM_DedicatedServer) return;
	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 2.0f, 0, 4.0f);
}

void ACXPawn::BeginPlay()
{
	Super::BeginPlay();
	UpdateMatchInputLock();
}

bool ACXPawn::IsMatchPlaying() const
{
	const ACXChatGameState* State = GetWorld() ? GetWorld()->GetGameState<ACXChatGameState>() : nullptr;
	return State && State->GetMatchState() == ECXMatchState::Playing;
}

void ACXPawn::UpdateMatchInputLock()
{
	const bool bLocked = !IsMatchPlaying();
	if (bLocked)
	{
		bHandbrakeHeld = false;
		ChaosVehicleMovement->SetThrottleInput(0.f);
		ChaosVehicleMovement->SetSteeringInput(0.f);
		ChaosVehicleMovement->SetHandbrakeInput(true);
	}
	else if (bMatchInputLocked)
	{
		// Release only the match lock; subsequent player handbrake input remains intact.
		ChaosVehicleMovement->SetHandbrakeInput(bHandbrakeHeld);
	}
	bMatchInputLocked = bLocked;
}
