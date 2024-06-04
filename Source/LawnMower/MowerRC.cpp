// Mower pawn class developed by Cody Wheeler.


#include "MowerRC.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"


AMowerRC::AMowerRC()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignComponentSubObjects();
	SetupComponentAttachments();
	SetComponentProperties();
}


void AMowerRC::CreateAndAssignComponentSubObjects()
{
	PhysicsBody = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsBody"));
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
	FrWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FRWheel"));
	FlWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FLWheel"));
	BrWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BRWheel"));
	BlWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BLWheel"));
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
}


void AMowerRC::SetupComponentAttachments()
{
	RootComponent = PhysicsBody;
	Body->SetupAttachment(RootComponent);
	Handle->SetupAttachment(RootComponent);
	FrWheel->SetupAttachment(RootComponent);
	FlWheel->SetupAttachment(RootComponent);
	BrWheel->SetupAttachment(RootComponent);
	BlWheel->SetupAttachment(RootComponent);
	CameraArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(CameraArm);
}


void AMowerRC::SetComponentProperties()
{
	PhysicsBody->SetBoxExtent(PhysicsBodyDimensions);
	PhysicsBody->SetGenerateOverlapEvents(false);
	PhysicsBody->SetSimulatePhysics(true);
	PhysicsBody->SetUseCCD(true);
	PhysicsBody->SetCollisionProfileName(TEXT("PhysicsActor"));

	CameraArm->SetRelativeRotation(DefaultLocalCameraArmRotation);
	CameraArm->SetUsingAbsoluteRotation(true);
	CameraArm->TargetArmLength = 250.0f;
	CameraArm->ProbeSize = 8.0f;
	CameraArm->bInheritPitch = false;
	CameraArm->bInheritYaw = false;
	CameraArm->bInheritRoll = false;

	SetMeshComponentCollisionAndLocation(Body, BodyPosition);
	SetMeshComponentCollisionAndLocation(Handle, HandlePosition);
	SetMeshComponentCollisionAndLocation(FrWheel, FrWheelPosition);
	SetMeshComponentCollisionAndLocation(FlWheel, FlWheelPosition);
	SetMeshComponentCollisionAndLocation(BrWheel, BrWheelPosition);
	SetMeshComponentCollisionAndLocation(BlWheel, BlWheelPosition);
}


void AMowerRC::SetMeshComponentCollisionAndLocation(UStaticMeshComponent* Mesh, const FVector& Location)
{
	if (!Mesh) return;

	Mesh->SetRelativeLocation(Location);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCollisionProfileName("NoCollision");
}


void AMowerRC::BeginPlay()
{
	Super::BeginPlay();

	SetPhysicsBodyProperties();
	SetCameraArmRotation();
	AddInputMappingContextToLocalPlayerSubsystem();
}


void AMowerRC::SetPhysicsBodyProperties()
{
	PhysicsBody->SetMassOverrideInKg(NAME_None, PhysicsBodyMass);
	PhysicsBody->SetCenterOfMass(PhysicsBodyCenterOfMass);

	PhysicsBodyWorldTransform = PhysicsBody->GetComponentTransform();
	LocationThisTick = PhysicsBodyWorldTransform.GetLocation();
}


void AMowerRC::SetCameraArmRotation()
{
	WorldCameraArmRotation = UKismetMathLibrary::TransformRotation(PhysicsBodyWorldTransform, LocalCameraArmRotation);

	CameraArm->SetWorldRotation(WorldCameraArmRotation);
}


void AMowerRC::AddInputMappingContextToLocalPlayerSubsystem() const
{
	APlayerController* PlayerController{};
	UEnhancedInputLocalPlayerSubsystem* Subsystem{};

	PlayerController = Cast<APlayerController>(Controller);
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (PlayerController && Subsystem) Subsystem->AddMappingContext(InputMappingContext, 0);
}


void AMowerRC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveCameraInputAction, ETriggerEvent::Triggered, this, &AMowerRC::MoveCamera);
		EnhancedInputComponent->BindAction(ResetCameraInputAction, ETriggerEvent::Triggered, this, &AMowerRC::ResetCamera);
		EnhancedInputComponent->BindAction(AccelerateInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Accelerate);
		EnhancedInputComponent->BindAction(PitchInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Pitch);
		EnhancedInputComponent->BindAction(BrakeInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Brake);
		EnhancedInputComponent->BindAction(SteerInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Steer);
		EnhancedInputComponent->BindAction(DriftInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Drift);
	}
}


void AMowerRC::MoveCamera(const FInputActionValue& Value) { RotatingCameraDirection = Value.Get<FVector2D>(); }
void AMowerRC::ResetCamera(const FInputActionValue& Value) { bCameraReset = Value.Get<bool>(); }
void AMowerRC::Accelerate(const FInputActionValue& Value) { AcceleratingDirection = Value.Get<float>(); }
void AMowerRC::Pitch(const FInputActionValue& Value) { PitchingDirection = Value.Get<float>(); };
void AMowerRC::Brake(const FInputActionValue& Value) { Braking = Value.Get<float>(); }
void AMowerRC::Steer(const FInputActionValue& Value) { Steering = Value.Get<float>(); }
void AMowerRC::Drift(const FInputActionValue& Value) { Drifting = Value.Get<float>(); }


void AMowerRC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// StartTickTimer();

	// Float();

	UpdateTransforms();

	UpdateSpeed();

	UpdateCameraRotation();

	UpdateHoveringForces(ForceRayCasts, ForceRayCastOrigins);

	UpdateGroundedMovementConditions();
	UpdateAccelerationRatio(DeltaTime);
	UpdateDriftingRatio(DeltaTime);
	UpdateAcceleratingDirection();
	ApplyAccelerationForce();
	ApplySteeringTorque();
	ApplyDriftingForce();

	UpdateAirTimeRatio(DeltaTime);
	ApplyAirTimeAntiGravitationalForce();
	ApplyAirTimePitch();
	ApplyAirTimeRoll();

	AddBrakingLinearDrag();
	AddAcceleratingAngularDrag();
	AddAirTimeAngularDrag();
	ApplyDrag();

	UpdateWheelSuspension(WheelRayCasts, WheelRayCastOrigins);
	UpdateWheelRotations(DeltaTime);

	DrawRayCastGroup(ForceRayCasts);
	// DrawRayCastGroup(WheelRayCasts);
	// DrawAcceleration();
	// DrawDrift();

	// LogMotionData(DeltaTime);

	ResetDrag();
	ResetPlayerInputData();

	// LogTickTime();
}


void AMowerRC::GetTickTime()
{
	TickTime = FPlatformTime::Seconds();
}


void AMowerRC::Float() const
{ 
	PhysicsBody->AddForce(FVector::UpVector * PhysicsBodyAntiGravitationalForce); 
}


void AMowerRC::UpdateTransforms()
{
	PhysicsBodyWorldTransform = PhysicsBody->GetComponentTransform();
	PhysicsBodyLocalTransform = PhysicsBody->GetRelativeTransform();

	PhysicsBodyLocation = PhysicsBodyWorldTransform.GetLocation();
	PhysicsBodyForwardVector = PhysicsBodyWorldTransform.GetUnitAxis(EAxis::Type::X);
	PhysicsBodyRightVector = PhysicsBodyWorldTransform.GetUnitAxis(EAxis::Type::Y);
	PhysicsBodyUpVector = PhysicsBodyWorldTransform.GetUnitAxis(EAxis::Type::Z);
}


void AMowerRC::UpdateSpeed()
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
}


void AMowerRC::UpdateCameraRotation()
{
	if (bCameraReset) LocalCameraArmRotation = DefaultLocalCameraArmRotation;
	else LocalCameraArmRotation += FRotator{ RotatingCameraDirection.Y, RotatingCameraDirection.X, 0.0 };

	ResetFullAxisRotations(LocalCameraArmRotation);

	if (LocalCameraArmRotation.Pitch > MaxLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MaxLocalCameraArmPitch;
	if (LocalCameraArmRotation.Pitch < -MaxLocalCameraArmPitch) LocalCameraArmRotation.Pitch = -MaxLocalCameraArmPitch;

	const FRotator LocalCameraArmRotationThisTick{ UKismetMathLibrary::InverseTransformRotation(PhysicsBodyLocalTransform, WorldCameraArmRotation) };

	if (LocalCameraArmRotationThisTick != LocalCameraArmRotation) SetCameraArmRotation();
}


void AMowerRC::ResetFullAxisRotations(FRotator& Rotator) const
{
	if (abs(Rotator.Pitch) >= RotationMaximum) Rotator.Pitch = 0.0;
	if (abs(Rotator.Yaw) >= RotationMaximum) Rotator.Yaw = 0.0;
	if (abs(Rotator.Roll) >= RotationMaximum) Rotator.Roll = 0.0;
}


void AMowerRC::UpdateHoveringForces(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins)
{
	if (ForceRayCastHitGround(RayCastGroup.Fr, LocalOrigins.Fr)) ApplyHoveringForce(RayCastGroup.Fr);
	if (ForceRayCastHitGround(RayCastGroup.Fl, LocalOrigins.Fl)) ApplyHoveringForce(RayCastGroup.Fl);
	if (ForceRayCastHitGround(RayCastGroup.Br, LocalOrigins.Br)) ApplyHoveringForce(RayCastGroup.Br);
	if (ForceRayCastHitGround(RayCastGroup.Bl, LocalOrigins.Bl)) ApplyHoveringForce(RayCastGroup.Bl);
}


bool AMowerRC::ForceRayCastHitGround(FHitResult& RayCast, const FVector& LocalOrigin)
{
	RayCast.Reset();

	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, LocalOrigin) };
	const FVector RayCastEnd{ RayCastStart + (-PhysicsBodyUpVector * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(RayCast, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


void AMowerRC::ApplyHoveringForce(FHitResult& RayCast)
{
	const float CompressionRatio{ 1.0f - RayCast.Time };
	const double Force{ PhysicsBodyMass * GravitationalAcceleration * CompressionRatio };

	PhysicsBody->AddForceAtLocation(RayCast.ImpactNormal * Force, RayCast.TraceStart);

	AddHoveringForceDrag(CompressionRatio);
}


void AMowerRC::AddHoveringForceDrag(const float CompressionRatio)
{
	float Drag{};

	++WheelsGrounded;

	if (CompressionRatio < CompressionRatioMinimum) Drag = MaxWheelDrag;
	else Drag = MaxWheelDrag / (CompressionRatio * WheelTotal);

	LinearDragArray.Add(Drag);
	AngularDragArray.Add(Drag);
}


void AMowerRC::UpdateGroundedMovementConditions()
{
	bMovingByAccumulatedAcceleration = AccelerationRatio != 0.0f;
	bAccelerating = AcceleratingDirection != 0.0f;
	bSteering = Steering != 0.0f;

	if (bAccelerating) AcceleratingDirection > 0.0f ? bLastAccelerationWasForward = true : bLastAccelerationWasForward = false;
}


void AMowerRC::UpdateAccelerationRatio(const float DeltaTime)
{
	if (!WheelsGrounded) return;

	if (bAccelerating && WheelsGrounded == 4) AccelerationRatio += AcceleratingDirection * DeltaTime;
	else DecayRatio(AccelerationRatio, AccelerationDecayRate, DeltaTime);

	LimitRatio(AccelerationRatio, AccelerationRatioMaximum);

	if (!Braking) return;

	float BrakingRate{};

	bAccelerating ? BrakingRate = AccelerationBrakingRate : BrakingRate = AccelerationBrakingDecayRate;

	if (AccelerationRatio > AccelerationRatioMinimumWhileBraking) AccelerationRatio += -Braking * BrakingRate * DeltaTime;
	if (AccelerationRatio < -AccelerationRatioMinimumWhileBraking) AccelerationRatio += Braking * BrakingRate * DeltaTime;
}


void AMowerRC::UpdateDriftingRatio(const float DeltaTime)
{
	if (Steering < 0.0f && DriftingRatio < 0.0f) DriftingRatio = -DriftingRatio;
	if (Steering > 0.0f && DriftingRatio > 0.0f) DriftingRatio = -DriftingRatio;

	if (!WheelsGrounded) return;

	if (!Drifting || !bMovingByAccumulatedAcceleration || Braking) DecayRatio(DriftingRatio, DriftingDecayRate, DeltaTime);
	
	const bool CanDrift{ Drifting && bSteering && bMovingByAccumulatedAcceleration && !Braking};

	if (CanDrift) DriftingRatio += -Steering * DriftingRate * DeltaTime;

	LimitRatio(DriftingRatio, DriftingRatioMaximum);
}


void AMowerRC::DecayRatio(float& Ratio, const float DecayRate, const float DeltaTime)
{
	if (Ratio < 0.0f) Ratio += DecayRate * DeltaTime;
	if (Ratio > 0.0f) Ratio -= DecayRate * DeltaTime;
	if (Ratio < 0.1f && Ratio > -0.1f) Ratio = 0.0f;
}


void AMowerRC::LimitRatio(float& Ratio, const float RatioMaximum)
{
	if (Ratio > RatioMaximum) Ratio = RatioMaximum;
	if (Ratio < -RatioMaximum) Ratio = -RatioMaximum;
}


void AMowerRC::UpdateAcceleratingDirection()
{
	AccelerationSurfaceImpact = PhysicsBodyLocation + (-PhysicsBodyUpVector * PhysicsBodyCenterOfMassOffset);
	AccelerationSurfaceNormal = PhysicsBodyForwardVector;

	if (AccelerationRatio < 0.0f) AccelerationSurfaceNormal = -AccelerationSurfaceNormal;
}


void AMowerRC::ApplyAccelerationForce()
{
	AccelerationForce = 0.0f;
	
	if (!WheelsGrounded) return;

	AccelerationForce = AccelerationForceMaximum * WheelsGrounded * abs(AccelerationRatio);
	
	PhysicsBody->AddForceAtLocation(AccelerationSurfaceNormal * AccelerationForce, AccelerationSurfaceImpact);
}


void AMowerRC::ApplySteeringTorque()
{
	if (!WheelsGrounded) return;

	double SteeringForce{ Steering * SteeringForceMaximum * AccelerationForceMaximum * WheelsGrounded };

	if (bMovingByAccumulatedAcceleration) SteeringForce *= AccelerationRatio;
	else SteeringForce *= SteeringForceOnSlopeRate * PhysicsBodySpeed;

	PhysicsBody->AddTorqueInDegrees(AccelerationSurfaceImpact + (PhysicsBodyUpVector * SteeringForce));
}


void AMowerRC::ApplyDriftingForce()
{
	DriftingForcePosition =  AccelerationSurfaceImpact + (-PhysicsBodyForwardVector * DriftingForcePositionOffset);

	if (!WheelsGrounded) return;
	
	const double AccelerationForceAtRatioMaximum{ AccelerationForceMaximum * AccelerationRatioMaximum * WheelTotal };
	const double AccelerationForceRatio{ AccelerationForce / AccelerationForceAtRatioMaximum };
	
	double DriftingForce{ DriftingForceMaximum * abs(DriftingRatio) * AccelerationForceRatio * WheelsGrounded };

	if (DriftingForce == 0.0) return; 

	if (AccelerationRatio < 0.0f) DriftingForce = -DriftingForce;

	if (DriftingRatio > 0.0f) PhysicsBody->AddForceAtLocation(PhysicsBodyRightVector * DriftingForce, DriftingForcePosition);
	if (DriftingRatio < 0.0f) PhysicsBody->AddForceAtLocation(-PhysicsBodyRightVector * DriftingForce, DriftingForcePosition);
}


void AMowerRC::UpdateAirTimeRatio(const float DeltaTime)
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


void AMowerRC::ApplyAirTimeAntiGravitationalForce()
{
	if (WheelsGrounded || !bAirTimeMinimumExceeded) return;
	
	FVector TrajectoryNormal{ FVector{ LocationThisTick - LocationLastTick}.GetSafeNormal() };

	double AntiGravitationalForce{ AirTimeAntiGravitationalForce * (AirTimeRatioMaxium - AirTimeRatio)};

	PhysicsBody->AddForce(-TrajectoryNormal * AntiGravitationalForce);

	// DrawDebugLine(GetWorld(), PhysicsBodyLocation, PhysicsBodyLocation + (TrajectoryNormal * 20.0), FColor::Green, false, 5.0f);
}


void AMowerRC::ApplyAirTimePitch()
{
	if (WheelsGrounded || !bAirTimeMinimumExceeded) return;

	if (PitchingDirection > 0.0f) PhysicsBody->AddTorqueInDegrees(PhysicsBodyLocation + (PhysicsBodyRightVector * AirTimePitchForce));
	if (PitchingDirection < 0.0f) PhysicsBody->AddTorqueInDegrees(PhysicsBodyLocation + (-PhysicsBodyRightVector * AirTimePitchForce));
}


void AMowerRC::ApplyAirTimeRoll()
{
	if (WheelsGrounded || !bAirTimeMinimumExceeded) return;

	if (Steering > 0.0f) PhysicsBody->AddTorqueInDegrees(PhysicsBodyLocation + (-PhysicsBodyForwardVector * AirTimeRollForce));
	if (Steering < 0.0f) PhysicsBody->AddTorqueInDegrees(PhysicsBodyLocation + (PhysicsBodyForwardVector * AirTimeRollForce));
}


void AMowerRC::AddBrakingLinearDrag()
{
	if (WheelsGrounded && Braking && !bMovingByAccumulatedAcceleration) LinearBrakingDrag += BrakingLinearDragRate;
	else LinearBrakingDrag = 0.0f;

	if (LinearBrakingDrag > BrakingLinearDragLimit) LinearBrakingDrag = BrakingLinearDragLimit;

	LinearDragArray.Add(LinearBrakingDrag);
}


void AMowerRC::AddAcceleratingAngularDrag()
{
	AngularDragArray.Add(AccelerationForce * WheelsGrounded * AcceleratingAngularDragRate);
}


void AMowerRC::AddAirTimeAngularDrag()
{
	if (!WheelsGrounded) AngularDragArray.Add(AirTimeAngularDrag);
}


void AMowerRC::ApplyDrag()
{
	for (float Drag : LinearDragArray) TotalLinearDrag += Drag;
	for (float Drag : AngularDragArray) TotalAngularDrag += Drag;

	PhysicsBody->SetLinearDamping(TotalLinearDrag);
	PhysicsBody->SetAngularDamping(TotalAngularDrag);
}


void AMowerRC::UpdateWheelSuspension(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins)
{
	SendWheelRayCast(FrWheel, RayCastGroup.Fr, LocalOrigins.Fr);
	SendWheelRayCast(FlWheel, RayCastGroup.Fl, LocalOrigins.Fl);
	SendWheelRayCast(BrWheel, RayCastGroup.Br, LocalOrigins.Br);
	SendWheelRayCast(BlWheel, RayCastGroup.Bl, LocalOrigins.Bl);
}


void AMowerRC::SendWheelRayCast(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin)
{
	RayCast.Reset();

	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, LocalOrigin) };
	const FVector WheelEnd{ WheelStart + (-PhysicsBodyUpVector * RayCastLength) };

	const bool Grounded{ GetWorld()->LineTraceSingleByChannel(RayCast, WheelStart, WheelEnd, ECC_GameTraceChannel1) };

	ApplyWheelSuspension(Wheel, RayCast, WheelStart, Grounded);
}


void AMowerRC::ApplyWheelSuspension(UStaticMeshComponent* Wheel, const FHitResult& RayCast, const FVector& WheelStart, const bool Grounded)
{
	if (!Wheel) return;
	
	double RayCastLengthDifference{};

	if (Grounded) RayCastLengthDifference = RayCastLength - RayCast.Distance;

	Wheel->SetWorldLocation(WheelStart + (PhysicsBodyUpVector * RayCastLengthDifference));
}


void AMowerRC::UpdateWheelRotations(const float DeltaTime)
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


double AMowerRC::GetWheelPitch(const double PitchRate, const float Ratio, const float RatioMaximum, const float DeltaTime) const
{
	double WheelPitch{ PitchRate * (Ratio / RatioMaximum) * DeltaTime };

	if (WheelPitch == 0.0) WheelPitch = -PhysicsBodySpeed * WheelOnSlopePitchRate;

	return WheelPitch;
}


float AMowerRC::GetDriftingRatioWithPitchDirection() const
{
	float DriftingRatioWithPitchDirection{ abs(DriftingRatio) + WheelPitchMinimumWhileDrifting };

	if (AccelerationRatio < 0.0f || !bLastAccelerationWasForward) DriftingRatioWithPitchDirection = -DriftingRatioWithPitchDirection;
	if (AccelerationRatio == 0.0f) DriftingRatioWithPitchDirection *= WheelPitchMinimumWhileDrifting;

	return DriftingRatioWithPitchDirection;
}


void AMowerRC::UpdateWheelPitch(FRotator& LocalRotation, const double WheelPitch)
{
	LocalRotation.Pitch += WheelPitch;

	ResetFullAxisRotations(LocalRotation);
}


void AMowerRC::UpdateWheelSteeringRatio(const float DeltaTime)
{
	if (!bSteering) DecayRatio(WheelSteeringRatio, WheelSteeringDecayRate, DeltaTime);

	if (bSteering) WheelSteeringRatio += Steering * WheelSteeringRate * DeltaTime;
		
	LimitRatio(WheelSteeringRatio, WheelSteeringRatioMaximum);
}


void AMowerRC::UpdateWheelYaw(FRotator& LocalRotation) const
{
	LocalRotation.Yaw = WheelSteeringMaximum * WheelSteeringRatio;
}


void AMowerRC::ApplyWheelRotation(UStaticMeshComponent* Wheel, const FRotator& LocalRotation) const
{
	if (!Wheel) return;
	
	Wheel->SetWorldRotation(UKismetMathLibrary::TransformRotation(PhysicsBodyWorldTransform, LocalRotation));
}


void AMowerRC::DrawRayCastGroup(const FRayCastGroup& RayCasts) const
{
	DrawRayCast(RayCasts.Fr);
	DrawRayCast(RayCasts.Fl);
	DrawRayCast(RayCasts.Br);
	DrawRayCast(RayCasts.Bl);
}


void AMowerRC::DrawRayCast(const FHitResult& RayCast) const
{
	if (!RayCast.bBlockingHit)
	{
		// DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.TraceEnd, FColor::Red);
		// DrawDebugSphere(GetWorld(), RayCast.TraceEnd, 1.0f, 6, FColor::Red);

		DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.TraceEnd, FColor::Red, false, 0.1f);
		DrawDebugSphere(GetWorld(), RayCast.TraceEnd, 1.0f, 6, FColor::Red, false, 0.1f);
	}
	else
	{
		// DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.ImpactPoint, FColor::Green);
		// DrawDebugSphere(GetWorld(), RayCast.ImpactPoint, 1.0f, 6, FColor::Green);

		DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.ImpactPoint, FColor::Green, false, 0.1f);
		DrawDebugSphere(GetWorld(), RayCast.ImpactPoint, 1.0f, 6, FColor::Green, false, 0.1f);
	}
}


void AMowerRC::DrawAcceleration() const
{
	const FVector CurrentAcceleration{ AccelerationSurfaceImpact + (AccelerationSurfaceNormal * abs(AccelerationRatio) * RayCastLength) };
	
	DrawDebugSphere(GetWorld(), AccelerationSurfaceImpact, 1.0f, 6, FColor::Orange);
	DrawDebugLine(GetWorld(), AccelerationSurfaceImpact, CurrentAcceleration, FColor::Yellow);
	DrawDebugSphere(GetWorld(), CurrentAcceleration, 1.0f, 6, FColor::Yellow);
}


void AMowerRC::DrawDrift() const
{
	FVector CurrentDrift{ DriftingForcePosition };
	
	if (DriftingRatio > 0.0f) CurrentDrift += (PhysicsBodyRightVector * abs(DriftingRatio) * RayCastLength);
	if (DriftingRatio < 0.0f) CurrentDrift += (- PhysicsBodyRightVector * abs(DriftingRatio) * RayCastLength);

	DrawDebugSphere(GetWorld(), DriftingForcePosition, 1.0f, 6, FColor::Purple);
	DrawDebugLine(GetWorld(), DriftingForcePosition, CurrentDrift, FColor::Magenta);
	DrawDebugSphere(GetWorld(), CurrentDrift, 1.0f, 6, FColor::Magenta);
}


void AMowerRC::LogMotionData(const float DeltaTime)
{
	UpdateTickCount(DeltaTime);

	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT(" "));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("Speed               %f"), abs(PhysicsBodySpeed));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationRatio   %f"), AccelerationRatio);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("DriftingRatio       %f"), DriftingRatio);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AirTimeRatio        %f"), AirTimeRatio);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("LinearDrag          %f"), TotalLinearDrag);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AngularDrag         %f"), TotalAngularDrag);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT(" "));
}


void AMowerRC::UpdateTickCount(const float DeltaTime)
{
	const float TickCountMultiplier{ 60.0f };
	
	TickCount += TickCountMultiplier * DeltaTime;

	if (TickCount >= 1.0f) TickCount = 0.0f;

	TickCount == 0.0f ? bTickReset = true : bTickReset = false;
}


void AMowerRC::ResetDrag()
{
	LinearDragArray.Reset();
	AngularDragArray.Reset();

	TotalLinearDrag = 0.01f;
	TotalAngularDrag = 0.0f;

	WheelsGrounded = 0;
}


void AMowerRC::ResetPlayerInputData()
{
	RotatingCameraDirection = FVector2D::Zero();
	bCameraReset = false;
	AcceleratingDirection = 0.0f;
	PitchingDirection = 0.0f;
	Braking = 0.0f;
	Steering = 0.0f;
	Drifting = 0.0f;

	bMovingByAccumulatedAcceleration = false;
	bAccelerating = false;
	bSteering = false;
}


void AMowerRC::LogTickTime()
{
	TickTime = FPlatformTime::Seconds() - TickTime;
	if (TickTime > LongestTickTime) LongestTickTime = TickTime;

	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("TickTime            %f"), TickTime);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("LongestTickTime     %f"), LongestTickTime);
}
