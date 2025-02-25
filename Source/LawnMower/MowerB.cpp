// Derived APawn class AMowerB by Cody Wheeler.


#include "MowerB.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MowerPlayerControllerA.h"
#include "NiagaraComponent.h"


AMowerB::AMowerB()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignComponentSubObjects();
	SetupComponentAttachments();
	SetComponentProperties();
}

void AMowerB::CreateAndAssignComponentSubObjects()
{
	PhysicsBody = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsBody"));
	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Blade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
	FrWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FRWheel"));
	FlWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FLWheel"));
	BrWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BRWheel"));
	BlWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BLWheel"));
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Emitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Emitter"));
	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	MovementAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("MovementAudio"));
	JumpAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("JumpAudio"));
	CrashAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("CrashAudio"));
	CutAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("CutAudio"));
}

void AMowerB::SetupComponentAttachments()
{
	RootComponent = PhysicsBody;
	Collider->SetupAttachment(RootComponent);
	Body->SetupAttachment(RootComponent);
	Blade->SetupAttachment(RootComponent);
	FrWheel->SetupAttachment(RootComponent);
	FlWheel->SetupAttachment(RootComponent);
	BrWheel->SetupAttachment(RootComponent);
	BlWheel->SetupAttachment(RootComponent);
	CameraArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(CameraArm);
	Emitter->SetupAttachment(RootComponent);
	EngineAudio->SetupAttachment(RootComponent);
	MovementAudio->SetupAttachment(RootComponent);
	JumpAudio->SetupAttachment(RootComponent);
	CrashAudio->SetupAttachment(RootComponent);
	CutAudio->SetupAttachment(RootComponent);
}

void AMowerB::SetComponentProperties()
{
	PhysicsBody->SetBoxExtent(PhysicsBodyDimensions);
	PhysicsBody->SetSimulatePhysics(true);
	PhysicsBody->SetUseCCD(true);
	PhysicsBody->SetCollisionProfileName(TEXT("PhysicsActor"));
	PhysicsBody->SetNotifyRigidBodyCollision(true);

	PhysicsBody->OnComponentHit.AddDynamic(this, &AMowerB::PlayCrashAudio);

	Collider->SetBoxExtent(ColliderDimensions);
	Collider->SetRelativeLocation(ColliderPosition);
	Collider->SetCollisionProfileName(TEXT("Custom..."));
	Collider->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Collider->SetCollisionObjectType(ECC_GameTraceChannel3);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	
	Collider->OnComponentBeginOverlap.AddDynamic(this, &AMowerB::StartEmitter);

	CameraArm->SetRelativeRotation(DefaultLocalCameraArmRotation);
	CameraArm->SetUsingAbsoluteRotation(true);
	CameraArm->TargetArmLength = 350.0f;
	CameraArm->ProbeSize = 8.0f;
	CameraArm->bInheritPitch = false;
	CameraArm->bInheritYaw = false;
	CameraArm->bInheritRoll = false;

	SetMeshComponentCollisionAndLocation(Body, BodyPosition);
	SetMeshComponentCollisionAndLocation(Blade, BladePosition);
	SetMeshComponentCollisionAndLocation(FrWheel, FrWheelPosition);
	SetMeshComponentCollisionAndLocation(FlWheel, FlWheelPosition);
	SetMeshComponentCollisionAndLocation(BrWheel, BrWheelPosition);
	SetMeshComponentCollisionAndLocation(BlWheel, BlWheelPosition);

	Emitter->bAutoActivate = false;
	JumpAudio->bAutoActivate = false;
	CrashAudio->bAutoActivate = false;
	CutAudio->bAutoActivate = false;
}

void AMowerB::SetMeshComponentCollisionAndLocation(UStaticMeshComponent* Mesh, const FVector& Location)
{
	if (!Mesh) return;

	Mesh->SetRelativeLocation(Location);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCollisionProfileName("NoCollision");
}

void AMowerB::PlayCrashAudio(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (CrashAudioTimer < CrashAudioTimerReady) return;
	
	CrashAudio->SetVolumeMultiplier(FMath::RandRange(0.75f, 1.00f));
	CrashAudio->SetPitchMultiplier(FMath::RandRange(0.85f, 1.35f));
	
	if (LastActorCrash != OtherActor->GetName()) CrashAudio->Activate();

	LastActorCrash = OtherActor->GetName();
	
	CrashAudioTimer = 0.0f;
}

void AMowerB::StartEmitter(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (CutAudioTimer == CutAudioTimerMaximum) PlayCutAudio();

	if (EmitterTime < EmitterTimeMaximum) return;

	EmitterTime = 0.0;

	Emitter->Activate(true);
}


void AMowerB::BeginPlay()
{
	Super::BeginPlay();

	SetPhysicsBodyProperties();
	SetCameraArmRotation();
	AddInputMappingContextToLocalPlayerSubsystem();
}

void AMowerB::SetPhysicsBodyProperties()
{
	PhysicsBody->SetMassOverrideInKg(NAME_None, PhysicsBodyMass);
	PhysicsBody->SetCenterOfMass(PhysicsBodyCenterOfMass);

	PhysicsBodyWorldTransform = PhysicsBody->GetComponentTransform();
	LocationThisTick = PhysicsBodyWorldTransform.GetLocation();

	RespawnLocation = PhysicsBodyWorldTransform.GetLocation();
}

void AMowerB::SetCameraArmRotation()
{
	WorldCameraArmRotation = UKismetMathLibrary::TransformRotation(PhysicsBodyWorldTransform, LocalCameraArmRotation);

	CameraArm->SetWorldRotation(WorldCameraArmRotation);
}

void AMowerB::AddInputMappingContextToLocalPlayerSubsystem() const
{
	APlayerController* PlayerController{};
	UEnhancedInputLocalPlayerSubsystem* Subsystem{};

	PlayerController = Cast<APlayerController>(Controller);
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (PlayerController && Subsystem) Subsystem->AddMappingContext(InputMappingContext, 0);
}

void AMowerB::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveCameraInputAction, ETriggerEvent::Triggered, this, &AMowerB::MoveCamera);
		EnhancedInputComponent->BindAction(ResetCameraInputAction, ETriggerEvent::Triggered, this, &AMowerB::ResetCamera);
		EnhancedInputComponent->BindAction(AccelerateInputAction, ETriggerEvent::Triggered, this, &AMowerB::Accelerate);
		EnhancedInputComponent->BindAction(PitchInputAction, ETriggerEvent::Triggered, this, &AMowerB::Pitch);
		EnhancedInputComponent->BindAction(BrakeInputAction, ETriggerEvent::Triggered, this, &AMowerB::Brake);
		EnhancedInputComponent->BindAction(SteerInputAction, ETriggerEvent::Triggered, this, &AMowerB::Steer);
		EnhancedInputComponent->BindAction(DriftInputAction, ETriggerEvent::Triggered, this, &AMowerB::Drift);
		EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &AMowerB::Jump);
		
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Triggered, this, &AMowerB::Pause);
		EnhancedInputComponent->BindAction(NavigateVerticallyAction, ETriggerEvent::Triggered, this, &AMowerB::NavigateVertically);
		EnhancedInputComponent->BindAction(NavigateHorizontallyAction, ETriggerEvent::Triggered, this, &AMowerB::NavigateHorizontally);
		EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Triggered, this, &AMowerB::Select);
		EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Triggered, this, &AMowerB::Cancel);
	}

	MowerController = Cast<AMowerPlayerControllerA>(UGameplayStatics::GetPlayerController(this, 0));
}

void AMowerB::MoveCamera(const FInputActionValue& Value) { RotatingCameraDirection = Value.Get<FVector2D>(); }
void AMowerB::ResetCamera(const FInputActionValue& Value) { bCameraReset = Value.Get<bool>(); }
void AMowerB::Accelerate(const FInputActionValue& Value) { AcceleratingDirection = Value.Get<float>(); }
void AMowerB::Pitch(const FInputActionValue& Value) { PitchingDirection = Value.Get<float>(); };
void AMowerB::Brake(const FInputActionValue& Value) { Braking = Value.Get<float>(); }
void AMowerB::Steer(const FInputActionValue& Value) { Steering = Value.Get<float>(); }
void AMowerB::Drift(const FInputActionValue& Value) { Drifting = Value.Get<float>(); }
void AMowerB::Jump(const FInputActionValue& Value) { Jumping = Value.Get<float>(); }

void AMowerB::Pause(const FInputActionValue& Value) 
{
	bGamePaused = !bGamePaused; 
	UGameplayStatics::SetGamePaused(this, bGamePaused);

	if (!MowerController) return;

	if(bGamePaused) MowerController->DisplayPauseMenu();
	else MowerController->HidePauseMenu();
}

void AMowerB::NavigateVertically(const FInputActionValue& Value) 
{
	if (!MowerController || !bGamePaused) return;

	MowerController->UpdateVerticalNavigation(Value.Get<float>());
}

void AMowerB::NavigateHorizontally(const FInputActionValue& Value) 
{
	if (!MowerController || !bGamePaused) return;

	MowerController->UpdateHorizontalNavigation(Value.Get<float>());
}

void AMowerB::Select(const FInputActionValue& Value)
{
	if (!MowerController || !bGamePaused) return;

	if (MowerController->SelectOption()) Pause(true);
}

void AMowerB::Cancel(const FInputActionValue& Value)
{
	if (!MowerController || !bGamePaused) return;

	if (MowerController->CancelOption()) Pause(true);
}


void AMowerB::Float() const { PhysicsBody->AddForce(FVector::UpVector * PhysicsBodyAntiGravitationalForce); }
float AMowerB::GetPhysicsBodyMass() const { return PhysicsBodyMass; }
UBoxComponent* AMowerB::GetPhysicsBody() { return PhysicsBody; }
void AMowerB::SetNewRespawnLocation(const FVector& NewLocation) { RespawnLocation = NewLocation; }


void AMowerB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// GetTickTime();

	UpdateTransforms();

	UpdateSpeed();

	UpdateCameraRotation();

	UpdateHoveringForces(ForceRayCasts, ForceRayCastOrigins);

	UpdateGroundedMovementConditions();
	UpdateAccelerationRatio(DeltaTime);
	UpdateDriftingRatio(DeltaTime);
	UpdateJumpReadyRatio(DeltaTime);
	UpdateJumpingRatio(DeltaTime);
	UpdateAcceleratingDirection();

	ApplyAccelerationForce();
	ApplySteeringTorque();
	ApplyDriftingForce();
	ApplyJumpingForce();

	UpdateAirTimeRatio(DeltaTime);

	// ApplyAirTimeAntiGravitationalForce();

	ApplyAirTimePitchTorque();
	ApplyAirTimeRollTorque();

	AddBrakingLinearDrag();
	AddAcceleratingAngularDrag();
	AddAirTimeAngularDrag();
	ApplyDrag();

	UpdateWheelSuspension(WheelRayCasts, WheelRayCastOrigins);
	UpdateWheelRotations(DeltaTime);
	ApplyBladeRotation(DeltaTime);
	UpdateVibratingPosition(DeltaTime);
	ApplyVibratingPosition();

	UpdateEmitterTime(DeltaTime);

	UpdateEngineAudioPitch();
	UpdateMovementAudioVolumeAndPitch();
	UpdateCrashAudioTimer(DeltaTime);
	UpdateCutAudioTimer(DeltaTime);

	// DrawRayCastGroup(ForceRayCasts);
	// DrawRayCastGroup(WheelRayCasts);
	// DrawAcceleration();
	// DrawDrift();

	// LogMotionData(DeltaTime);

	ResetDrag();
	ResetPlayerInputData();

	// LogTickTime();
}

void AMowerB::GetTickTime()
{
	TickTime = FPlatformTime::Seconds();
}

void AMowerB::UpdateTransforms()
{
	if (PhysicsBodyLocation.Z < PhysicsBodyRespawnHeight) Respawn();
	
	PhysicsBodyWorldTransform = PhysicsBody->GetComponentTransform();
	PhysicsBodyLocalTransform = PhysicsBody->GetRelativeTransform();

	PhysicsBodyLocation = PhysicsBodyWorldTransform.GetLocation();
	PhysicsBodyForwardVector = PhysicsBodyWorldTransform.GetUnitAxis(EAxis::Type::X);
	PhysicsBodyRightVector = PhysicsBodyWorldTransform.GetUnitAxis(EAxis::Type::Y);
	PhysicsBodyUpVector = PhysicsBodyWorldTransform.GetUnitAxis(EAxis::Type::Z);

	VibratingDirection = PhysicsBodyUpVector;
}

void AMowerB::Respawn()
{
	SetActorLocation(RespawnLocation);
}

void AMowerB::UpdateSpeed()
{
	LocationLastTick = LocationThisTick;
	LocationThisTick = PhysicsBodyLocation;
	PhysicsBodySpeed = FVector::Dist(LocationThisTick, LocationLastTick);

	const FVector FrontLocationLastTick{ LocationLastTick + (PhysicsBodyForwardVector * DriftingForcePositionOffset) };
	const FVector RearLocationLastTick{ LocationLastTick + (-PhysicsBodyForwardVector * DriftingForcePositionOffset) };

	const double DistanceFromFrontLocationLastTick{ FVector::Dist(LocationThisTick, FrontLocationLastTick) };
	const double DistanceFromRearLocationLastTick{ FVector::Dist(LocationThisTick, RearLocationLastTick) };

	const bool MovingForward{ DistanceFromFrontLocationLastTick < DistanceFromRearLocationLastTick };

	if (!MovingForward) PhysicsBodySpeed = -PhysicsBodySpeed;

	HoverSpeedMultiplier = 1.0;

	if (bAcceleratingLastTick && abs(PhysicsBodySpeed) > 5.0) HoverSpeedMultiplier += (abs(PhysicsBodySpeed) * 0.1);
}

void AMowerB::UpdateCameraRotation()
{
	if (bCameraReset) LocalCameraArmRotation = DefaultLocalCameraArmRotation;
	else LocalCameraArmRotation += FRotator{ RotatingCameraDirection.Y, RotatingCameraDirection.X, 0.0 };

	ResetFullAxisRotations(LocalCameraArmRotation);

	if (LocalCameraArmRotation.Pitch > MaxLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MaxLocalCameraArmPitch;
	if (LocalCameraArmRotation.Pitch < -MaxLocalCameraArmPitch) LocalCameraArmRotation.Pitch = -MaxLocalCameraArmPitch;

	const FRotator LocalCameraArmRotationThisTick{ UKismetMathLibrary::InverseTransformRotation(PhysicsBodyLocalTransform, WorldCameraArmRotation) };

	if (LocalCameraArmRotationThisTick != LocalCameraArmRotation) SetCameraArmRotation();
}

void AMowerB::ResetFullAxisRotations(FRotator& Rotator) const
{
	if (abs(Rotator.Pitch) >= RotationMaximum) Rotator.Pitch = 0.0;
	if (abs(Rotator.Yaw) >= RotationMaximum) Rotator.Yaw = 0.0;
	if (abs(Rotator.Roll) >= RotationMaximum) Rotator.Roll = 0.0;
}

void AMowerB::UpdateHoveringForces(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins)
{
	if (ForceRayCastHitGround(RayCastGroup.Fr, LocalOrigins.Fr)) ApplyHoveringForce(RayCastGroup.Fr);
	if (ForceRayCastHitGround(RayCastGroup.Fl, LocalOrigins.Fl)) ApplyHoveringForce(RayCastGroup.Fl);
	if (ForceRayCastHitGround(RayCastGroup.Br, LocalOrigins.Br)) ApplyHoveringForce(RayCastGroup.Br);
	if (ForceRayCastHitGround(RayCastGroup.Bl, LocalOrigins.Bl)) ApplyHoveringForce(RayCastGroup.Bl);
}

bool AMowerB::ForceRayCastHitGround(FHitResult& RayCast, const FVector& LocalOrigin)
{
	RayCast.Reset();

	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, LocalOrigin) };
	const FVector RayCastEnd{ RayCastStart + (-PhysicsBodyUpVector * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(RayCast, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}

void AMowerB::ApplyHoveringForce(FHitResult& RayCast)
{
	const float CompressionRatio{ 1.0f - RayCast.Time };

	const double Force{ PhysicsBodyMass * GravitationalAcceleration * HoverSpeedMultiplier * CompressionRatio };

	PhysicsBody->AddForceAtLocation(RayCast.ImpactNormal * Force, RayCast.TraceStart);

	AddHoveringForceDrag(CompressionRatio);
}

void AMowerB::AddHoveringForceDrag(const float CompressionRatio)
{
	float Drag{};

	++WheelsGrounded;

	if (CompressionRatio < CompressionRatioMinimum) Drag = MaxWheelDrag;
	else Drag = MaxWheelDrag / (CompressionRatio * WheelTotal);

	LinearDragArray.Add(Drag);
	AngularDragArray.Add(Drag);
}

void AMowerB::UpdateGroundedMovementConditions()
{
	bMovingByAccumulatedAcceleration = AccelerationRatio != 0.0f;
	bAccelerating = AcceleratingDirection != 0.0f;
	bSteering = Steering != 0.0f;

	bAcceleratingLastTick = bAccelerating;

	if (bAccelerating) AcceleratingDirection > 0.0f ? bLastAccelerationWasForward = true : bLastAccelerationWasForward = false;
}

void AMowerB::UpdateAccelerationRatio(const float DeltaTime)
{
	if (bAccelerating) AccelerationRatio += AcceleratingDirection * AccelerationRatioRate * DeltaTime;
	else DecayRatio(AccelerationRatio, AccelerationDecayRate, DeltaTime);

	LimitRatio(AccelerationRatio, AccelerationRatioMaximum);

	if (!Braking) return;

	float BrakingRate{};

	bAccelerating ? BrakingRate = AccelerationBrakingRate : BrakingRate = AccelerationBrakingDecayRate;

	if (AccelerationRatio > AccelerationRatioMinimumWhileBraking) AccelerationRatio += -Braking * BrakingRate * DeltaTime;
	if (AccelerationRatio < -AccelerationRatioMinimumWhileBraking) AccelerationRatio += Braking * BrakingRate * DeltaTime;
}

void AMowerB::UpdateDriftingRatio(const float DeltaTime)
{
	if (Steering < 0.0f && DriftingRatio < 0.0f) DriftingRatio = -DriftingRatio;
	if (Steering > 0.0f && DriftingRatio > 0.0f) DriftingRatio = -DriftingRatio;

	if (!WheelsGrounded) return;

	if (!Drifting || !bMovingByAccumulatedAcceleration || Braking) DecayRatio(DriftingRatio, DriftingDecayRate, DeltaTime);

	const bool CanDrift{ Drifting && bSteering && bMovingByAccumulatedAcceleration && !Braking };

	if (CanDrift) DriftingRatio += -Steering * DriftingRate * DeltaTime;

	LimitRatio(DriftingRatio, DriftingRatioMaximum);
}

void AMowerB::UpdateJumpReadyRatio(const float DeltaTime)
{
	if (WheelsGrounded) JumpReadyRatio += JumpReadyRate * DeltaTime;
	else
	{
		JumpReadyRatio = 0.0f;
		bJumpReady = false;
	}

	LimitRatio(JumpReadyRatio, JumpReadyRatioMaximum);

	if (JumpReadyRatio == JumpReadyRatioMaximum) bJumpReady = true;
}

void AMowerB::UpdateJumpingRatio(const float DeltaTime)
{
	if (!WheelsGrounded && !bStartedJumping) return;

	if (WheelsGrounded && Jumping && !bStartedJumping && bJumpReady)
	{
		JumpingForceDirection = PhysicsBodyUpVector;
		bStartedJumping = true;

		PlayJumpAudio();
	}

	if (Jumping && bStartedJumping) JumpingRatio += JumpingRate * DeltaTime;

	LimitRatio(JumpingRatio, JumpingRatioMaximum);

	if ((!Jumping && !WheelsGrounded) || JumpingRatio == JumpingRatioMaximum)
	{
		JumpingRatio = 0.0f;
		bJumpReady = false;
		bStartedJumping = false;
	}
}

void AMowerB::DecayRatio(float& Ratio, const float DecayRate, const float DeltaTime)
{
	if (Ratio < 0.0f) Ratio += DecayRate * DeltaTime;
	if (Ratio > 0.0f) Ratio -= DecayRate * DeltaTime;
	if (Ratio < 0.1f && Ratio > -0.1f) Ratio = 0.0f;
}

void AMowerB::LimitRatio(float& Ratio, const float RatioMaximum)
{
	if (Ratio > RatioMaximum) Ratio = RatioMaximum;
	if (Ratio < -RatioMaximum) Ratio = -RatioMaximum;
}

void AMowerB::UpdateAcceleratingDirection()
{
	AccelerationSurfaceImpact = PhysicsBodyLocation + (-PhysicsBodyUpVector * PhysicsBodyCenterOfMassOffset);
	AccelerationSurfaceNormal = PhysicsBodyForwardVector;

	if (AccelerationRatio < 0.0f) AccelerationSurfaceNormal = -AccelerationSurfaceNormal;
}

void AMowerB::ApplyAccelerationForce()
{
	AccelerationForce = 0.0f;

	if (!WheelsGrounded) return;

	AccelerationForce = AccelerationForceMaximum * abs(AccelerationRatio) * TotalLinearDragLastTick;

	PhysicsBody->AddForceAtLocation(AccelerationSurfaceNormal * AccelerationForce, AccelerationSurfaceImpact);
}

void AMowerB::ApplySteeringTorque()
{
	if (!WheelsGrounded) return;

	double SteeringForce{ Steering * SteeringForceMultiplier * AccelerationForceMaximum * TotalLinearDragLastTick };

	if (bMovingByAccumulatedAcceleration) SteeringForce *= AccelerationRatio;
	else SteeringForce *= SteeringForceOnSlopeRate * PhysicsBodySpeed;

	PhysicsBody->AddTorqueInDegrees(AccelerationSurfaceImpact + (PhysicsBodyUpVector * SteeringForce));
}

void AMowerB::ApplyDriftingForce()
{
	DriftingForcePosition = AccelerationSurfaceImpact + (-PhysicsBodyForwardVector * DriftingForcePositionOffset);

	if (!WheelsGrounded) return;

	const double AccelerationForceAtRatioMaximum{ AccelerationForceMaximum * AccelerationRatioMaximum * AccelerationMaxLinearDrag };
	const double AccelerationForceRatio{ AccelerationForce / AccelerationForceAtRatioMaximum };

	double DriftingForce{ DriftingForceMaximum * abs(DriftingRatio) * AccelerationForceRatio * TotalLinearDragLastTick };

	if (DriftingForce == 0.0) return;

	if (AccelerationRatio < 0.0f) DriftingForce = -DriftingForce;

	if (DriftingRatio > 0.0f) PhysicsBody->AddForceAtLocation(PhysicsBodyRightVector * DriftingForce, DriftingForcePosition);
	if (DriftingRatio < 0.0f) PhysicsBody->AddForceAtLocation(-PhysicsBodyRightVector * DriftingForce, DriftingForcePosition);
}

void AMowerB::ApplyJumpingForce()
{
	if (!bStartedJumping) return;

	const double JumpingForce{ JumpingForceMaximum * (JumpingRatioMaximum - JumpingRatio) };

	PhysicsBody->AddForce(JumpingForceDirection * JumpingForce);
}

void AMowerB::UpdateAirTimeRatio(const float DeltaTime)
{
	if (WheelsGrounded)
	{
		AirTimeRatio = 0.0f;
		return;
	}

	AirTimeRatio += AirTimeRatioIncreaseRate * DeltaTime;

	LimitRatio(AirTimeRatio, AirTimeRatioMaxium);

	bAirTimeMinimumExceeded = AirTimeRatio >= AirTimeMinimum;
}

void AMowerB::ApplyAirTimeAntiGravitationalForce()
{
	if (WheelsGrounded || !bAirTimeMinimumExceeded) return;

	FVector TrajectoryNormal{ FVector{ LocationThisTick - LocationLastTick}.GetSafeNormal() };

	double AntiGravitationalForce{ AirTimeAntiGravitationalForce * (AirTimeRatioMaxium - AirTimeRatio) };

	PhysicsBody->AddForce(-TrajectoryNormal * AntiGravitationalForce);
}

void AMowerB::ApplyAirTimePitchTorque()
{
	if (WheelsGrounded || !bAirTimeMinimumExceeded) return;

	if (PitchingDirection > 0.0f) PhysicsBody->AddTorqueInDegrees(PhysicsBodyLocation + (PhysicsBodyRightVector * AirTimePitchTorque));
	if (PitchingDirection < 0.0f) PhysicsBody->AddTorqueInDegrees(PhysicsBodyLocation + (-PhysicsBodyRightVector * AirTimePitchTorque));
}

void AMowerB::ApplyAirTimeRollTorque()
{
	if (WheelsGrounded || !bAirTimeMinimumExceeded) return;

	double AirTimeRollTorqueTotal{ AirTimeRollTorque };

	if (abs(PhysicsBodySpeed) < 0.5) AirTimeRollTorqueTotal *= StalledRollTorqueMultiplier;

	if (Steering > 0.0f) PhysicsBody->AddTorqueInDegrees(PhysicsBodyLocation + (-PhysicsBodyForwardVector * AirTimeRollTorqueTotal));
	if (Steering < 0.0f) PhysicsBody->AddTorqueInDegrees(PhysicsBodyLocation + (PhysicsBodyForwardVector * AirTimeRollTorqueTotal));
}

void AMowerB::AddBrakingLinearDrag()
{
	if (WheelsGrounded && Braking && !bMovingByAccumulatedAcceleration) LinearBrakingDrag += BrakingLinearDragRate;
	else LinearBrakingDrag = 0.0f;

	if (LinearBrakingDrag > BrakingLinearDragLimit) LinearBrakingDrag = BrakingLinearDragLimit;

	LinearDragArray.Add(LinearBrakingDrag);
}

void AMowerB::AddAcceleratingAngularDrag()
{
	AngularDragArray.Add(AccelerationForce * TotalLinearDragLastTick * AcceleratingAngularDragRate);
}

void AMowerB::AddAirTimeAngularDrag()
{
	if (!WheelsGrounded) AngularDragArray.Add(AirTimeAngularDrag);
}

void AMowerB::ApplyDrag()
{
	for (float Drag : LinearDragArray) TotalLinearDrag += Drag;
	for (float Drag : AngularDragArray) TotalAngularDrag += Drag;

	PhysicsBody->SetLinearDamping(TotalLinearDrag);
	PhysicsBody->SetAngularDamping(TotalAngularDrag);

	TotalLinearDragLastTick = TotalLinearDrag;
}

void AMowerB::UpdateWheelSuspension(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins)
{
	SendWheelRayCast(FrWheel, RayCastGroup.Fr, LocalOrigins.Fr);
	SendWheelRayCast(FlWheel, RayCastGroup.Fl, LocalOrigins.Fl);
	SendWheelRayCast(BrWheel, RayCastGroup.Br, LocalOrigins.Br);
	SendWheelRayCast(BlWheel, RayCastGroup.Bl, LocalOrigins.Bl);
}

void AMowerB::SendWheelRayCast(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin)
{
	RayCast.Reset();

	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, LocalOrigin) };
	const FVector WheelEnd{ WheelStart + (-PhysicsBodyUpVector * RayCastLength) };

	const bool Grounded{ GetWorld()->LineTraceSingleByChannel(RayCast, WheelStart, WheelEnd, ECC_GameTraceChannel1) };

	ApplyWheelSuspension(Wheel, RayCast, WheelStart, Grounded);
}

void AMowerB::ApplyWheelSuspension(UStaticMeshComponent* Wheel, const FHitResult& RayCast, const FVector& WheelStart, const bool Grounded)
{
	if (!Wheel) return;

	double RayCastLengthDifference{};

	if (Grounded) RayCastLengthDifference = RayCastLength - RayCast.Distance;

	Wheel->SetWorldLocation(WheelStart + (PhysicsBodyUpVector * RayCastLengthDifference));
}

void AMowerB::UpdateWheelRotations(const float DeltaTime)
{
	const double AcceleratingPitch{ GetWheelPitch(WheelAcceleratingPitchRate, AccelerationRatio, AccelerationRatioMaximum, DeltaTime) };
	const double DriftingPitch{ GetWheelPitch(WheelDriftingPitchRate, GetDriftingRatioWithPitchDirection(), DriftingRatioMaximum, DeltaTime) };

	UpdateWheelPitch(LocalFrontWheelRotations, AcceleratingPitch);
	UpdateWheelPitch(LocalRearWheelRotations, AcceleratingPitch + DriftingPitch);

	UpdateWheelSteeringRatio(DeltaTime);
	UpdateWheelYaw(LocalFrontWheelRotations);

	ApplyWheelRotation(FrWheel, LocalFrontWheelRotations);
	ApplyWheelRotation(FlWheel, LocalFrontWheelRotations);
	ApplyWheelRotation(BrWheel, LocalRearWheelRotations);
	ApplyWheelRotation(BlWheel, LocalRearWheelRotations);
}

double AMowerB::GetWheelPitch(const double PitchRate, const float Ratio, const float RatioMaximum, const float DeltaTime) const
{
	double WheelPitch{ PitchRate * (Ratio / RatioMaximum) * DeltaTime };

	if (WheelPitch == 0.0 && WheelsGrounded) WheelPitch = -PhysicsBodySpeed * WheelOnSlopePitchRate;

	return WheelPitch;
}

float AMowerB::GetDriftingRatioWithPitchDirection() const
{
	float DriftingRatioWithPitchDirection{ abs(DriftingRatio) + WheelPitchMinimumWhileDrifting };

	if (AccelerationRatio < 0.0f || !bLastAccelerationWasForward) DriftingRatioWithPitchDirection = -DriftingRatioWithPitchDirection;
	if (AccelerationRatio == 0.0f) DriftingRatioWithPitchDirection *= WheelPitchMinimumWhileDrifting;

	return DriftingRatioWithPitchDirection;
}

void AMowerB::UpdateWheelPitch(FRotator& LocalRotation, const double WheelPitch)
{
	LocalRotation.Pitch += WheelPitch;

	ResetFullAxisRotations(LocalRotation);
}

void AMowerB::UpdateWheelSteeringRatio(const float DeltaTime)
{
	if (!bSteering) DecayRatio(WheelSteeringRatio, WheelSteeringDecayRate, DeltaTime);

	if (bSteering) WheelSteeringRatio += Steering * WheelSteeringRate * DeltaTime;

	LimitRatio(WheelSteeringRatio, WheelSteeringRatioMaximum);
}

void AMowerB::UpdateWheelYaw(FRotator& LocalRotation) const
{
	LocalRotation.Yaw = WheelSteeringMaximum * WheelSteeringRatio;
}

void AMowerB::ApplyWheelRotation(UStaticMeshComponent* Wheel, const FRotator& LocalRotation) const
{
	if (!Wheel) return;

	if (!WheelsGrounded && PitchingDirection != 0.0 && !AccelerationRatio) return;

	Wheel->SetWorldRotation(UKismetMathLibrary::TransformRotation(PhysicsBodyWorldTransform, LocalRotation));
}

void AMowerB::ApplyBladeRotation(const float DeltaTime)
{
	LocalBladeRotation.Yaw += BladeRotationRate * DeltaTime;

	ResetFullAxisRotations(LocalBladeRotation);

	Blade->SetWorldRotation(UKismetMathLibrary::TransformRotation(PhysicsBodyWorldTransform, LocalBladeRotation));
}

void AMowerB::UpdateVibratingPosition(const float DeltaTime)
{
	VibratingPosition += VibratingDirectionRate * DeltaTime;

	if (VibratingPosition >= VibratingPositionMaximum)
	{
		VibratingPosition = VibratingPositionMaximum;
		VibratingDirectionRate = -VibratingDirectionRate;
		VibratingDirection = -VibratingDirection;
	}

	if (VibratingPosition <= 0.0)
	{
		VibratingPosition = 0.0;
		VibratingDirectionRate = -VibratingDirectionRate;
		VibratingDirection = -VibratingDirection;
	}
}

void AMowerB::ApplyVibratingPosition()
{
	const FVector NewPositionOffset{ 0.0, 0.0, VibratingPosition };

	Body->SetRelativeLocation(BodyPosition + NewPositionOffset);
	Blade->SetRelativeLocation(BladePosition + NewPositionOffset);
}

void AMowerB::UpdateEmitterTime(const float DeltaTime)
{
	EmitterTime += DeltaTime;

	if (EmitterTime >= EmitterTimeMaximum) EmitterTime = EmitterTimeMaximum;
}

void AMowerB::UpdateEngineAudioPitch()
{
	EngineAudio->SetPitchMultiplier(EnginePitchMultiplierDefault + (abs(AccelerationRatio) / EnginePitchAccelerationDivisor));
}

void AMowerB::UpdateMovementAudioVolumeAndPitch()
{
	MovementAudio->SetVolumeMultiplier(0.0f);
	
	if (!WheelsGrounded || !AccelerationRatio && abs(PhysicsBodySpeed) < 0.1) return;

	MovementAudio->SetVolumeMultiplier(0.25f + (abs(AccelerationRatio) / 12.0));
}

void AMowerB::PlayJumpAudio()
{
	JumpAudio->SetPitchMultiplier(FMath::RandRange(0.85f, 1.10f));
	JumpAudio->Activate();
}

void AMowerB::UpdateCrashAudioTimer(const float DeltaTime)
{
	CrashAudioTimer += DeltaTime;

	if (CrashAudioTimer >= CrashAudioTimerReady) CrashAudioTimer = CrashAudioTimerReady;
}

void AMowerB::UpdateCutAudioTimer(const float DeltaTime)
{
	CutAudioTimer += DeltaTime;

	if (CutAudioTimer >= CutAudioTimerMaximum) CutAudioTimer = CutAudioTimerMaximum;
}

void AMowerB::PlayCutAudio()
{
	CutAudio->Activate(true);

	CutAudioTimer = 0.0f;
}

void AMowerB::DrawRayCastGroup(const FRayCastGroup& RayCasts) const
{
	DrawRayCast(RayCasts.Fr);
	DrawRayCast(RayCasts.Fl);
	DrawRayCast(RayCasts.Br);
	DrawRayCast(RayCasts.Bl);
}

void AMowerB::DrawRayCast(const FHitResult& RayCast) const
{
	if (!RayCast.bBlockingHit)
	{
		DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.TraceEnd, FColor::Red);
		DrawDebugSphere(GetWorld(), RayCast.TraceEnd, 1.0f, 6, FColor::Red);
	}
	else
	{
		DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.ImpactPoint, FColor::Green);
		DrawDebugSphere(GetWorld(), RayCast.ImpactPoint, 1.0f, 6, FColor::Green);
	}
}

void AMowerB::DrawAcceleration() const
{
	const FVector CurrentAcceleration{ AccelerationSurfaceImpact + (AccelerationSurfaceNormal * abs(AccelerationRatio) * RayCastLength) };

	DrawDebugSphere(GetWorld(), AccelerationSurfaceImpact, 1.0f, 6, FColor::Orange);
	DrawDebugLine(GetWorld(), AccelerationSurfaceImpact, CurrentAcceleration, FColor::Yellow);
	DrawDebugSphere(GetWorld(), CurrentAcceleration, 1.0f, 6, FColor::Yellow);
}

void AMowerB::DrawDrift() const
{
	FVector CurrentDrift{ DriftingForcePosition };

	if (DriftingRatio > 0.0f) CurrentDrift += (PhysicsBodyRightVector * abs(DriftingRatio) * RayCastLength);
	if (DriftingRatio < 0.0f) CurrentDrift += (-PhysicsBodyRightVector * abs(DriftingRatio) * RayCastLength);

	DrawDebugSphere(GetWorld(), DriftingForcePosition, 1.0f, 6, FColor::Purple);
	DrawDebugLine(GetWorld(), DriftingForcePosition, CurrentDrift, FColor::Magenta);
	DrawDebugSphere(GetWorld(), CurrentDrift, 1.0f, 6, FColor::Magenta);
}

void AMowerB::LogMotionData(const float DeltaTime)
{
	UpdateTickCount(DeltaTime);

	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT(" "));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("HoverSpeedMulti     %f"), HoverSpeedMultiplier);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationForce   %f"), AccelerationForce);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("Speed               %f"), abs(PhysicsBodySpeed));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationRatio   %f"), AccelerationRatio);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("DriftingRatio       %f"), DriftingRatio);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AirTimeRatio        %f"), AirTimeRatio);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("LinearDrag          %f"), TotalLinearDrag);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AngularDrag         %f"), TotalAngularDrag);
}

void AMowerB::UpdateTickCount(const float DeltaTime)
{
	const float TickCountMultiplier{ 60.0f };

	TickCount += TickCountMultiplier * DeltaTime;

	if (TickCount >= 1.0f) TickCount = 0.0f;

	TickCount == 0.0f ? bTickReset = true : bTickReset = false;
}

void AMowerB::ResetDrag()
{
	LinearDragArray.Reset();
	AngularDragArray.Reset();

	TotalLinearDrag = 0.01f;
	TotalAngularDrag = 0.0f;

	WheelsGrounded = 0;
}

void AMowerB::ResetPlayerInputData()
{
	RotatingCameraDirection = FVector2D::Zero();
	bCameraReset = false;
	AcceleratingDirection = 0.0f;
	PitchingDirection = 0.0f;
	Braking = 0.0f;
	Steering = 0.0f;
	Drifting = 0.0f;
	Jumping = 0.0f;

	bMovingByAccumulatedAcceleration = false;
	bAccelerating = false;
	bSteering = false;
}

void AMowerB::LogTickTime()
{
	TickTime = FPlatformTime::Seconds() - TickTime;
	if (TickTime > LongestTickTime) LongestTickTime = TickTime;

	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("TickTime            %f"), TickTime);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("LongestTickTime     %f"), LongestTickTime);
}
