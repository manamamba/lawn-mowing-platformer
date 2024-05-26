// Player controlled lawn mower class by Cody Wheeler


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
	FRWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FRWheel"));
	FLWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FLWheel"));
	BRWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BRWheel"));
	BLWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BLWheel"));
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
}


void AMowerRC::SetupComponentAttachments()
{
	RootComponent = PhysicsBody;
	Body->SetupAttachment(RootComponent);
	Handle->SetupAttachment(RootComponent);
	FRWheel->SetupAttachment(RootComponent);
	FLWheel->SetupAttachment(RootComponent);
	BRWheel->SetupAttachment(RootComponent);
	BLWheel->SetupAttachment(RootComponent);
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
	SetMeshComponentCollisionAndLocation(FRWheel, FRWheelPosition);
	SetMeshComponentCollisionAndLocation(FLWheel, FLWheelPosition);
	SetMeshComponentCollisionAndLocation(BRWheel, BRWheelPosition);
	SetMeshComponentCollisionAndLocation(BLWheel, BLWheelPosition);
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

	AddInputMappingContextToLocalPlayerSubsystem();
	SetPhysicsBodyProperties();
	SetCameraArmWorldRotation();
}


void AMowerRC::AddInputMappingContextToLocalPlayerSubsystem() const
{
	APlayerController* PlayerController{};
	UEnhancedInputLocalPlayerSubsystem* Subsystem{};

	PlayerController = Cast<APlayerController>(Controller);
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (PlayerController && Subsystem) Subsystem->AddMappingContext(InputMappingContext, 0);
}


void AMowerRC::SetPhysicsBodyProperties()
{
	PhysicsBody->SetMassOverrideInKg(NAME_None, PhysicsBodyMass);
	PhysicsBody->SetCenterOfMass(PhysicsBodyCenterOfMass);

	PhysicsBodyWorldTransform = PhysicsBody->GetComponentTransform();
}


void AMowerRC::SetCameraArmWorldRotation()
{
	WorldCameraArmRotation = UKismetMathLibrary::TransformRotation(PhysicsBodyWorldTransform, LocalCameraArmRotation);

	CameraArm->SetWorldRotation(WorldCameraArmRotation);
}


void AMowerRC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveCameraInputAction, ETriggerEvent::Triggered, this, &AMowerRC::MoveCamera);
		EnhancedInputComponent->BindAction(ResetCameraInputAction, ETriggerEvent::Triggered, this, &AMowerRC::ResetCamera);
		EnhancedInputComponent->BindAction(AccelerateInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Accelerate);
		EnhancedInputComponent->BindAction(BrakeInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Brake);
		EnhancedInputComponent->BindAction(SteerInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Steer);
	}
}


void AMowerRC::MoveCamera(const FInputActionValue& Value) { RotatingCameraDirection = Value.Get<FVector2D>(); }
void AMowerRC::ResetCamera(const FInputActionValue& Value) { CameraReset = Value.Get<bool>(); }
void AMowerRC::Accelerate(const FInputActionValue& Value) { AcceleratingDirection = Value.Get<float>(); }
void AMowerRC::Brake(const FInputActionValue& Value) { Braking = Value.Get<float>(); }
void AMowerRC::Steer(const FInputActionValue& Value) { Steering = Value.Get<float>(); }


void PhysicsTick(float SubstepDeltaTime)
{


}


void AMowerRC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetTickRate(DeltaTime);
	TickCounter(DeltaTime);

	// FloatMower();

	UpdateAccelerationData(ForceRayCasts, DeltaTime);
	DecayAcceleration(DeltaTime);
	ApplyAcceleration();

	ApplySteeringTorque(DeltaTime);

	ResetDragForces();

	UpdatePhysicsBodyPositionData();

	UpdateCameraRotation();

	SendForceRayCasts(ForceRayCasts, ForceRayCastOrigins);
	SendWheelRayCasts(WheelRayCasts, WheelRayCastOrigins);

	// DrawRayCasts(ForceRayCasts);
	// DrawRayCasts(WheelRayCasts);
	// DrawAcceleration();

	AddBrakingDrag(DeltaTime);
	AddAirTimeAngularDrag();
	AddAcceleratingAngularDrag();
	ApplyDragForces();
}


void AMowerRC::SetTickRate(float DeltaTime) { TickRate = (1 / DeltaTime) / 60; }


void AMowerRC::TickCounter(float DeltaTime)
{
	TickCount += 1.0 * DeltaTime;

	if (TickCount > 1.0) TickCount = 0.0f;

	TickCount == 0.0f ? TickReset = true : TickReset = false;

	if (TickReset) UE_LOG(LogTemp, Warning, TEXT(" "));
}


void AMowerRC::FloatMower() const { PhysicsBody->AddForce(FVector::UpVector * PhysicsBodyAntiGravitationalForce); }


void AMowerRC::UpdateAccelerationData(const RayCastGroup& RayCastGroup, float DeltaTime)
{
	AccelerationSurfaceImpact = PhysicsBodyLocation + (-PhysicsBodyUpVector * PhysicsBodyCenterOfMassOffset);
	AccelerationSurfaceNormal = PhysicsBodyForwardVector;

	if (WheelsGrounded && !Braking) AccelerationRatio += AcceleratingDirection * DeltaTime;

	if (WheelsGrounded && Braking && AccelerationRatio < 0.0f) AccelerationRatio += Braking * DeltaTime;
	if (WheelsGrounded && Braking && AccelerationRatio > 0.0f) AccelerationRatio += -Braking * DeltaTime;

	if (AccelerationRatio > AccelerationRatioMaximum) AccelerationRatio = AccelerationRatioMaximum;
	if (AccelerationRatio < -AccelerationRatioMaximum) AccelerationRatio = -AccelerationRatioMaximum;

	AccelerationForce = AccelerationForceMaximum * TotalLinearDragForce * AccelerationRatio;

	if (AccelerationForce < 0.0f) AccelerationSurfaceNormal = -AccelerationSurfaceNormal;
}


void AMowerRC::DecayAcceleration(float DeltaTime)
{
	if (!AcceleratingDirection || (Braking && WheelsGrounded))
	{
		if (AccelerationRatio < 0.0f) AccelerationRatio += AcceleratingDecayRate * DeltaTime;
		if (AccelerationRatio > 0.0f) AccelerationRatio -= AcceleratingDecayRate * DeltaTime;
		if (AccelerationRatio < 0.1f && AccelerationRatio > -0.1f) AccelerationRatio = 0.0f;
	}

	AcceleratingDirection = 0.0f;
}


void AMowerRC::ApplyAcceleration() const
{
	if (AccelerationForce) PhysicsBody->AddForceAtLocation(AccelerationSurfaceNormal * abs(AccelerationForce), AccelerationSurfaceImpact);

	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationRatio   %f"), AccelerationRatio);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationForce   %f"), AccelerationForce);
}


void AMowerRC::ApplySteeringTorque(float DeltaTime)
{
	const double SteeringForce{ SteeringForceMultiplier * Steering * WheelsGrounded * AccelerationRatio * AccelerationForceMaximum };
	const FVector SteeringTorque{ PhysicsBodyLocation + (PhysicsBodyUpVector * SteeringForce) };

	if (Steering && WheelsGrounded && AccelerationRatio) PhysicsBody->AddTorqueInDegrees(SteeringTorque);
	
	if (Steering && TickReset) DrawDebugSphere(GetWorld(), PhysicsBodyLocation, 10.0, 6, FColor::Orange, false, 7.5f);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("SteeringForce      %f"), SteeringForce);

	Steering = 0.0f;
}


void AMowerRC::ResetDragForces()
{
	LinearDragForces.Reset();
	AngularDragForces.Reset();

	TotalLinearDragForce = 0.01f;
	TotalAngularDragForce = 0.0f;

	WheelsGrounded = 0.0f;
}


void AMowerRC::UpdatePhysicsBodyPositionData()
{
	PhysicsBodyWorldTransform = PhysicsBody->GetComponentTransform();
	PhysicsBodyLocalTransform = PhysicsBody->GetRelativeTransform();

	PhysicsBodyLocation = PhysicsBody->GetComponentLocation();
	PhysicsBodyUpVector = PhysicsBody->GetUpVector();
	PhysicsBodyForwardVector = PhysicsBody->GetForwardVector();
	PhysicsBodyRightVector = PhysicsBody->GetRightVector();

	LocationLastTick = LocationThisTick;
	LocationThisTick = PhysicsBodyLocation;
	const double Speed{ abs(FVector::Dist(LocationThisTick, LocationLastTick)) * TickRate };
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("Speed               %f"), Speed);
}


void AMowerRC::UpdateCameraRotation()
{	
	if (CameraReset) LocalCameraArmRotation = DefaultLocalCameraArmRotation;
	else LocalCameraArmRotation += FRotator{ RotatingCameraDirection.Y, RotatingCameraDirection.X, 0.0 };

	if (LocalCameraArmRotation.Pitch > MaxLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MaxLocalCameraArmPitch;
	if (LocalCameraArmRotation.Pitch < MinLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MinLocalCameraArmPitch;

	FRotator LocalCameraArmRotationThisTick{ UKismetMathLibrary::InverseTransformRotation(PhysicsBodyLocalTransform, WorldCameraArmRotation) };
	
	if (LocalCameraArmRotationThisTick != LocalCameraArmRotation) SetCameraArmWorldRotation();

	RotatingCameraDirection = FVector2D::Zero();
	CameraReset = false;
}


void AMowerRC::SendForceRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins)
{
	if (RayCastHit(RayCastGroup.FR, LocalOrigins.FR)) AddForcesOnRayCastHit(RayCastGroup.FR);
	if (RayCastHit(RayCastGroup.FL, LocalOrigins.FL)) AddForcesOnRayCastHit(RayCastGroup.FL);
	if (RayCastHit(RayCastGroup.BR, LocalOrigins.BR)) AddForcesOnRayCastHit(RayCastGroup.BR);
	if (RayCastHit(RayCastGroup.BL, LocalOrigins.BL)) AddForcesOnRayCastHit(RayCastGroup.BL);
}


bool AMowerRC::RayCastHit(FHitResult& RayCast, const FVector& LocalOrigin)
{
	RayCast.Reset();
	
	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, LocalOrigin) };
	const FVector RayCastEnd{ RayCastStart + (-PhysicsBodyUpVector * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(RayCast, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


void AMowerRC::AddForcesOnRayCastHit(FHitResult& RayCast)
{
	const float CompressionRatio{ 1.0f - RayCast.Time };
	const double Force{ PhysicsBodyMass * GravitationalAcceleration * CompressionRatio };

	PhysicsBody->AddForceAtLocation(RayCast.ImpactNormal * Force, RayCast.TraceStart);
	
	AddDragForceOnRayCastHit(CompressionRatio);
}


void AMowerRC::AddDragForceOnRayCastHit(float CompressionRatio)
{
	float DragForce{};

	++WheelsGrounded;

	if (CompressionRatio < CompressionRatioMinimum) DragForce = MaxWheelDragForce;
	else DragForce = MaxWheelDragForce / (CompressionRatio * WheelTotal);
	
	LinearDragForces.Add(DragForce);
	AngularDragForces.Add(DragForce);
}


void AMowerRC::SendWheelRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins)
{
	ApplySuspensionOnWheel(FRWheel, RayCastGroup.FR, LocalOrigins.FR);
	ApplySuspensionOnWheel(FLWheel, RayCastGroup.FL, LocalOrigins.FL);
	ApplySuspensionOnWheel(BRWheel, RayCastGroup.BR, LocalOrigins.BR);
	ApplySuspensionOnWheel(BLWheel, RayCastGroup.BL, LocalOrigins.BL);
}


void AMowerRC::ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin)
{
	RayCast.Reset();

	double RayCastLengthDifference{};

	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, LocalOrigin) };
	const FVector WheelEnd{ WheelStart + (-PhysicsBodyUpVector * RayCastLength) };
	const bool Grounded{ GetWorld()->LineTraceSingleByChannel(RayCast, WheelStart, WheelEnd, ECC_GameTraceChannel1) };

	if (Grounded) RayCastLengthDifference = RayCastLength - RayCast.Distance;

	Wheel->SetWorldLocation(WheelStart + (PhysicsBodyUpVector * RayCastLengthDifference));
}


// Wheel Animation Functions


void AMowerRC::DrawRayCasts(RayCastGroup& RayCasts) const
{
	DrawRayCast(RayCasts.FR);
	DrawRayCast(RayCasts.FL);
	DrawRayCast(RayCasts.BR);
	DrawRayCast(RayCasts.BL);
}


void AMowerRC::DrawRayCast(const FHitResult& RayCast) const
{
	if (!RayCast.bBlockingHit)
	{
		DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.TraceEnd, FColor::Red);
		DrawDebugSphere(GetWorld(), RayCast.TraceEnd, 1.0, 6, FColor::Red);

		return;
	}
	
	DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.ImpactPoint, FColor::Green);
	DrawDebugSphere(GetWorld(), RayCast.ImpactPoint, 1.0, 6, FColor::Green);
}


void AMowerRC::DrawAcceleration() const
{
	const FVector DrawStart{ AccelerationSurfaceImpact };
	const FVector DrawEnd{ AccelerationSurfaceImpact + (AccelerationSurfaceNormal * abs(AccelerationRatio) * RayCastLength) };

	DrawDebugSphere(GetWorld(), DrawStart, 1.0f, 6, FColor::Orange);
	DrawDebugLine(GetWorld(), DrawStart, DrawEnd, FColor::Orange);
	DrawDebugSphere(GetWorld(), DrawEnd, 1.0f, 6, FColor::Yellow);
}


void AMowerRC::AddBrakingDrag(float DeltaTime)
{
	if (WheelsGrounded && Braking && !AccelerationRatio) LinearBrakingDrag += LinearBrakingDragMultiplier * DeltaTime;
	else LinearBrakingDrag = 0.0f;

	if (LinearBrakingDrag > LinearBrakingDragLimit) LinearBrakingDrag = LinearBrakingDragLimit;

	// if (WheelsGrounded && Braking && Steering && AccelerationRatio)
	// AddAccelerationAngularDrag() is adding around 20 at top speed
	// subtract up to that limit based on 
	// do not want to go below 8 for stable hover
	// AngularDragForceMultiplier 0.00002
	// AngularDragForces.Add(AngularBrakingDrag);

	LinearDragForces.Add(LinearBrakingDrag);

	Braking = 0.0f;
}


void AMowerRC::AddAirTimeAngularDrag() { if (!WheelsGrounded) AngularDragForces.Add(AngularAirTimeDrag); }


void AMowerRC::AddAcceleratingAngularDrag() { AngularDragForces.Add(abs(AccelerationForce) * WheelsGrounded * AngularDragForceMultiplier); }


void AMowerRC::ApplyDragForces()
{
	for (float Force : LinearDragForces) TotalLinearDragForce += Force;
	for (float Force : AngularDragForces) TotalAngularDragForce += Force;

	PhysicsBody->SetLinearDamping(TotalLinearDragForce);
	PhysicsBody->SetAngularDamping(TotalAngularDragForce);

	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("=========DragForces========="));
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("LinearBraking       %f"), LinearBrakingDrag);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AngularAcceleration %f"), abs(AccelerationForce) * WheelsGrounded * AngularDragForceMultiplier);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("TotalLinear         %f"), TotalLinearDragForce);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("TotalAngular        %f"), TotalAngularDragForce);

	if (TotalLinearDragForce == 0.01f) TotalLinearDragForce = 0.0f;
}
