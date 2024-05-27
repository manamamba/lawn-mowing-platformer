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


void AMowerRC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickCounter(DeltaTime);

	// FloatMower();

	UpdateAccelerationData(ForceRayCasts, DeltaTime);
	DecayAcceleration(DeltaTime);
	ApplyAccelerationForce();

	UpdateSteeringForceVariance(DeltaTime);
	ApplySteeringForce(SteeringForce);

	ResetDragForces();

	UpdatePhysicsBodyPositionData(DeltaTime);
	UpdatePhysicsBodyVelocity(DeltaTime);
	UpdateCameraRotation();

	SendForceRayCasts(ForceRayCasts, ForceRayCastOrigins);
	SendWheelRayCasts(WheelRayCasts, WheelRayCastOrigins);

	// DrawRayCasts(ForceRayCasts);
	// DrawRayCasts(WheelRayCasts);
	// DrawAcceleration();

	AddBrakingLinearDrag(DeltaTime);
	AddAirTimeAngularDrag();
	AddAcceleratingAngularDrag();
	AddMostlyGroundedVarianceAngularDrag(DeltaTime);
	ApplyDragForces();

	ResetPlayerInputData();
}


void AMowerRC::TickCounter(float DeltaTime)
{
	const float TickCountMultiplier{ 8.0f };
	
	TickCount += TickCountMultiplier * DeltaTime;

	if (TickCount > 1.0f) TickCount = 0.0f;

	TickCount == 0.0f ? TickReset = true : TickReset = false;
}


void AMowerRC::FloatMower() const 
{ 
	PhysicsBody->AddForce(FVector::UpVector * PhysicsBodyAntiGravitationalForce); 
}


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
}


void AMowerRC::ApplyAccelerationForce() const
{
	if (AccelerationForce) PhysicsBody->AddForceAtLocation(AccelerationSurfaceNormal * abs(AccelerationForce), AccelerationSurfaceImpact);
}


void AMowerRC::UpdateSteeringForceVariance(float DeltaTime)
{
	if (DeltaTime < 0.02f) return;

	const float AccelerationPercentage{ abs(AccelerationRatio) / AccelerationRatioMaximum };

	SteeringForceVariance = (abs(AccelerationRatio) - 0.1f) + FMath::Pow(AccelerationPercentage, 2.0f);
}


void AMowerRC::ApplySteeringForce(double Force)
{
	if (Steering == 0.0f || AccelerationRatio == 0.0f || !WheelsGrounded) return;

	const FVector FrontSteeringWorldPosition{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, FrontSteeringLocalPosition) };
	const FVector BackSteeringWorldPosition{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, BackSteeringLocalPosition) };

	const double TotalSteeringForce{ Steering * AccelerationRatio * WheelsGrounded * Force * SteeringForceVariance };

	PhysicsBody->AddForceAtLocation(PhysicsBodyRightVector * TotalSteeringForce, FrontSteeringWorldPosition);
	PhysicsBody->AddForceAtLocation(-PhysicsBodyRightVector * TotalSteeringForce, BackSteeringWorldPosition);

	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("TotalSteeringForce %f"), TotalSteeringForce);

	SteeringForceVariance = 1.0f;
}


// braking drift


void AMowerRC::ResetDragForces()
{
	LinearDragForces.Reset();
	AngularDragForces.Reset();

	TotalLinearDragForce = 0.01f;
	TotalAngularDragForce = 0.0f;

	WheelsGrounded = 0.0f;
}


void AMowerRC::UpdatePhysicsBodyPositionData(float DeltaTime)
{
	PhysicsBodyWorldTransform = PhysicsBody->GetComponentTransform();
	PhysicsBodyLocalTransform = PhysicsBody->GetRelativeTransform();

	PhysicsBodyLocation = PhysicsBody->GetComponentLocation();
	PhysicsBodyUpVector = PhysicsBody->GetUpVector();
	PhysicsBodyForwardVector = PhysicsBody->GetForwardVector();
	PhysicsBodyRightVector = PhysicsBody->GetRightVector();
}


void AMowerRC::UpdatePhysicsBodyVelocity(float DeltaTime)
{
	const float TickRate = (1.0f / DeltaTime) / 60.0f;

	LocationLastTick = LocationThisTick;
	LocationThisTick = PhysicsBodyLocation;
	PhysicsBodyVelocity = abs(FVector::Dist(LocationThisTick, LocationLastTick)) * TickRate;
}


void AMowerRC::UpdateCameraRotation()
{
	if (CameraReset) LocalCameraArmRotation = DefaultLocalCameraArmRotation;
	else LocalCameraArmRotation += FRotator{ RotatingCameraDirection.Y, RotatingCameraDirection.X, 0.0 };

	if (LocalCameraArmRotation.Pitch > MaxLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MaxLocalCameraArmPitch;
	if (LocalCameraArmRotation.Pitch < MinLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MinLocalCameraArmPitch;

	FRotator LocalCameraArmRotationThisTick{ UKismetMathLibrary::InverseTransformRotation(PhysicsBodyLocalTransform, WorldCameraArmRotation) };

	if (LocalCameraArmRotationThisTick != LocalCameraArmRotation) SetCameraArmWorldRotation();
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
		DrawDebugSphere(GetWorld(), RayCast.TraceEnd, 1.0f, 6, FColor::Red);

		return;
	}

	DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.ImpactPoint, FColor::Green);
	DrawDebugSphere(GetWorld(), RayCast.ImpactPoint, 1.0f, 6, FColor::Green);
}


void AMowerRC::DrawAcceleration() const
{
	const FVector DrawStart{ AccelerationSurfaceImpact };
	const FVector DrawEnd{ AccelerationSurfaceImpact + (AccelerationSurfaceNormal * abs(AccelerationRatio) * RayCastLength) };

	DrawDebugSphere(GetWorld(), DrawStart, 1.0f, 6, FColor::Orange);
	DrawDebugLine(GetWorld(), DrawStart, DrawEnd, FColor::Orange);
	DrawDebugSphere(GetWorld(), DrawEnd, 1.0f, 6, FColor::Yellow);
}


void AMowerRC::AddBrakingLinearDrag(float DeltaTime)
{
	if (WheelsGrounded && Braking && !AccelerationRatio) LinearBrakingDrag += BrakingLinearDragMultiplier * DeltaTime;
	else LinearBrakingDrag = 0.0f;

	if (LinearBrakingDrag > BrakingLinearDragLimit) LinearBrakingDrag = BrakingLinearDragLimit;

	LinearDragForces.Add(LinearBrakingDrag);
}


void AMowerRC::AddAirTimeAngularDrag() 
{ 
	if (!WheelsGrounded) AngularDragForces.Add(AirTimeAngularDrag);
}


void AMowerRC::AddAcceleratingAngularDrag() 
{ 
	AngularDragForces.Add(abs(AccelerationForce) * WheelsGrounded * AcceleratingAngularDragMultiplier);
}


void AMowerRC::AddMostlyGroundedVarianceAngularDrag(float DeltaTime)
{
	if (DeltaTime < 0.02) return;

	if (WheelsGrounded > 0 && WheelsGrounded < 4) AngularDragForces.Add(WheelsGrounded * SteeringVarianceAngularDrag);
}

void AMowerRC::ApplyDragForces()
{
	for (float Force : LinearDragForces) TotalLinearDragForce += Force;
	for (float Force : AngularDragForces) TotalAngularDragForce += Force;

	PhysicsBody->SetLinearDamping(TotalLinearDragForce);
	PhysicsBody->SetAngularDamping(TotalAngularDragForce);

	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT(" "));
	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT("PhysicsBodyVelocity %f"), PhysicsBodyVelocity);
	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationForce   %f"), AccelerationForce);
	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationRatio   %f"), AccelerationRatio);
	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT("==================="));
	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT("LinearBraking       %f"), LinearBrakingDrag);
	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AngularAcceleration %f"), abs(AccelerationForce) * WheelsGrounded * AcceleratingAngularDragMultiplier);
	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT("TotalLinear         %f"), TotalLinearDragForce);
	// if (TickReset) UE_LOG(LogTemp, Warning, TEXT("TotalAngular        %f"), TotalAngularDragForce);

	if (TotalLinearDragForce == 0.01f) TotalLinearDragForce = 0.0f;
}


void AMowerRC::ResetPlayerInputData()
{
	RotatingCameraDirection = FVector2D::Zero();
	CameraReset = false;
	AcceleratingDirection = 0.0f;
	Braking = 0.0f;
	Steering = 0.0f;
}
