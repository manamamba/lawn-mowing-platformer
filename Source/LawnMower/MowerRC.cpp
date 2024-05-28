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

	SetPhysicsBodyProperties();
	SetCameraArmWorldRotation();
	AddInputMappingContextToLocalPlayerSubsystem();
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

	// FloatMower();

	UpdateTransforms();
	UpdateSpeed();
	UpdateCameraRotation();

	SendForceRayCasts(ForceRayCasts, ForceRayCastOrigins);

	UpdateAccelerationRatio(DeltaTime);
	UpdateAcceleratingDirection();
	ApplyAccelerationForce();
	ApplySteeringTorque();
	ApplyDriftingForce(DeltaTime);

	AddBrakingLinearDrag();
	AddAcceleratingAngularDrag();
	AddAirTimeAngularDrag();
	ApplyDrag();

	SendWheelSuspensionRayCasts(WheelRayCasts, WheelRayCastOrigins);

	LogData(DeltaTime);

	ResetDrag();
	ResetPlayerInputData();

	// DrawRayCastGroup(ForceRayCasts);
	// DrawRayCastGroup(WheelRayCasts);
	// DrawAcceleration();
}


void AMowerRC::FloatMower() const
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
	PhysicsBodySpeed = abs(FVector::Dist(LocationThisTick, LocationLastTick));
}


void AMowerRC::UpdateCameraRotation()
{
	if (CameraReset) LocalCameraArmRotation = DefaultLocalCameraArmRotation;
	else LocalCameraArmRotation += FRotator{ RotatingCameraDirection.Y, RotatingCameraDirection.X, 0.0 };

	if (LocalCameraArmRotation.Pitch > MaxLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MaxLocalCameraArmPitch;
	if (LocalCameraArmRotation.Pitch < MinLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MinLocalCameraArmPitch;

	const FRotator LocalCameraArmRotationThisTick{ UKismetMathLibrary::InverseTransformRotation(PhysicsBodyLocalTransform, WorldCameraArmRotation) };

	if (LocalCameraArmRotationThisTick != LocalCameraArmRotation) SetCameraArmWorldRotation();
}


void AMowerRC::SendForceRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins)
{
	if (ForceRayCastHit(RayCastGroup.FR, LocalOrigins.FR)) AddForcesOnRayCastHit(RayCastGroup.FR);
	if (ForceRayCastHit(RayCastGroup.FL, LocalOrigins.FL)) AddForcesOnRayCastHit(RayCastGroup.FL);
	if (ForceRayCastHit(RayCastGroup.BR, LocalOrigins.BR)) AddForcesOnRayCastHit(RayCastGroup.BR);
	if (ForceRayCastHit(RayCastGroup.BL, LocalOrigins.BL)) AddForcesOnRayCastHit(RayCastGroup.BL);
}


bool AMowerRC::ForceRayCastHit(FHitResult& RayCast, const FVector& LocalOrigin)
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

	AddDragOnRayCastHit(CompressionRatio);
}


void AMowerRC::AddDragOnRayCastHit(float CompressionRatio)
{
	float Drag{};

	++WheelsGrounded;

	if (CompressionRatio < CompressionRatioMinimum) Drag = MaxWheelDrag;
	else Drag = MaxWheelDrag / (CompressionRatio * WheelTotal);

	LinearDragArray.Add(Drag);
	AngularDragArray.Add(Drag);
}


void AMowerRC::UpdateAccelerationRatio(float DeltaTime)
{
	if (AcceleratingDirection == 0.0f)
	{
		if (AccelerationRatio < 0.0f) AccelerationRatio += AccelerationDecayRate * DeltaTime;
		if (AccelerationRatio > 0.0f) AccelerationRatio -= AccelerationDecayRate * DeltaTime;
		if (AccelerationRatio < 0.1f && AccelerationRatio > -0.1f) AccelerationRatio = 0.0f;
	}

	if (!WheelsGrounded) return;

	if (AcceleratingDirection != 0.0f) AccelerationRatio += AcceleratingDirection * DeltaTime;

	if (AccelerationRatio > AccelerationRatioMaximum) AccelerationRatio = AccelerationRatioMaximum;
	if (AccelerationRatio < -AccelerationRatioMaximum) AccelerationRatio = -AccelerationRatioMaximum;

	if (Braking)
	{
		if (AccelerationRatio > AccelerationRatioBrakingMinimum) AccelerationRatio += -Braking * AccelerationBrakingRate * DeltaTime;
		if (AccelerationRatio < -AccelerationRatioBrakingMinimum) AccelerationRatio += Braking * AccelerationBrakingRate * DeltaTime;
	}	
}


void AMowerRC::UpdateAcceleratingDirection()
{
	AccelerationSurfaceImpact = PhysicsBodyLocation + (-PhysicsBodyUpVector * PhysicsBodyCenterOfMassOffset);
	AccelerationSurfaceNormal = PhysicsBodyForwardVector;

	if (AccelerationRatio < 0.0f) AccelerationSurfaceNormal = -AccelerationSurfaceNormal;
}


void AMowerRC::ApplyAccelerationForce()
{
	if (!WheelsGrounded) AccelerationForce = 0.0f;
	else AccelerationForce = AccelerationForceMaximum * WheelsGrounded * abs(AccelerationRatio);
	
	PhysicsBody->AddForceAtLocation(AccelerationSurfaceNormal * AccelerationForce, AccelerationSurfaceImpact);
}


void AMowerRC::ApplySteeringTorque()
{
	if (Steering == 0.0f || AccelerationRatio == 0.0f || !WheelsGrounded) return;

	SteeringForce = Steering * SteeringTorque * WheelsGrounded * AccelerationRatio * AccelerationForceMaximum;

	PhysicsBody->AddTorqueInDegrees(AccelerationSurfaceImpact + (PhysicsBodyUpVector * SteeringForce));
}


void AMowerRC::ApplyDriftingForce(float DeltaTime)
{
	if (Braking && Steering != 0.0f) DriftingRatio += Steering * DeltaTime;







	const FVector DriftingForcePosition{ AccelerationSurfaceImpact + (-PhysicsBodyForwardVector * DriftingForcePositionOffset) };

	if (Steering < 0.0f) PhysicsBody->AddForceAtLocation(PhysicsBodyRightVector * DriftingRatio * DriftingForceMaximum, DriftingForcePosition);
	if (Steering > 0.0f) PhysicsBody->AddForceAtLocation(-PhysicsBodyRightVector * DriftingRatio * DriftingForceMaximum, DriftingForcePosition);
}


void AMowerRC::AddBrakingLinearDrag()
{
	if (WheelsGrounded && Braking && AccelerationRatio == 0.0f) LinearBrakingDrag += BrakingLinearDragIncreaseRate;
	else LinearBrakingDrag = 0.0f;

	if (LinearBrakingDrag > BrakingLinearDragLimit) LinearBrakingDrag = BrakingLinearDragLimit;

	LinearDragArray.Add(LinearBrakingDrag);
}


void AMowerRC::AddAcceleratingAngularDrag()
{
	AngularDragArray.Add(AccelerationForce * WheelsGrounded * AcceleratingAngularDragMultiplier);
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


void AMowerRC::SendWheelSuspensionRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins)
{
	SetWheelSuspension(FRWheel, RayCastGroup.FR, LocalOrigins.FR);
	SetWheelSuspension(FLWheel, RayCastGroup.FL, LocalOrigins.FL);
	SetWheelSuspension(BRWheel, RayCastGroup.BR, LocalOrigins.BR);
	SetWheelSuspension(BLWheel, RayCastGroup.BL, LocalOrigins.BL);
}


void AMowerRC::SetWheelSuspension(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin)
{
	RayCast.Reset();

	double RayCastLengthDifference{};

	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyWorldTransform, LocalOrigin) };
	const FVector WheelEnd{ WheelStart + (-PhysicsBodyUpVector * RayCastLength) };
	const bool Grounded{ GetWorld()->LineTraceSingleByChannel(RayCast, WheelStart, WheelEnd, ECC_GameTraceChannel1) };

	if (Grounded) RayCastLengthDifference = RayCastLength - RayCast.Distance;

	Wheel->SetWorldLocation(WheelStart + (PhysicsBodyUpVector * RayCastLengthDifference));
}


// Wheel Animations


void AMowerRC::LogData(float DeltaTime)
{
	UpdateTickCount(DeltaTime);

	if (TickReset) UE_LOG(LogTemp, Warning, TEXT(" "));
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("==================="));
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("Speed               %f"), PhysicsBodySpeed);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationForce   %f"), AccelerationForce);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationRatio   %f"), AccelerationRatio);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("==================="));
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("BrakingDrag         %f"), LinearBrakingDrag);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationDrag    %f"), AccelerationForce * WheelsGrounded * AcceleratingAngularDragMultiplier);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("LinearDrag          %f"), TotalLinearDrag);
	if (TickReset) UE_LOG(LogTemp, Warning, TEXT("AngularDrag         %f"), TotalAngularDrag);
}


void AMowerRC::UpdateTickCount(float DeltaTime)
{
	TickCount += TickCountMultiplier * DeltaTime;

	if (TickCount > 1.0f) TickCount = 0.0f;

	TickCount == 0.0f ? TickReset = true : TickReset = false;
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
	CameraReset = false;
	AcceleratingDirection = 0.0f;
	Braking = 0.0f;
	Steering = 0.0f;
}


void AMowerRC::DrawRayCastGroup(const RayCastGroup& RayCasts) const
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
	}
	else
	{
		DrawDebugLine(GetWorld(), RayCast.TraceStart, RayCast.ImpactPoint, FColor::Green);
		DrawDebugSphere(GetWorld(), RayCast.ImpactPoint, 1.0f, 6, FColor::Green);
	}
}


void AMowerRC::DrawAcceleration() const
{
	const FVector CurrentAcceleration{ AccelerationSurfaceImpact + (AccelerationSurfaceNormal * abs(AccelerationRatio) * RayCastLength) };
	
	DrawDebugSphere(GetWorld(), AccelerationSurfaceImpact, 1.0f, 6, FColor::Orange);
	DrawDebugLine(GetWorld(), AccelerationSurfaceImpact, CurrentAcceleration, FColor::Yellow);
	DrawDebugSphere(GetWorld(), CurrentAcceleration, 1.0f, 6, FColor::Yellow);

	// const FVector BrakingForceStart{ AccelerationSurfaceImpact + (-PhysicsBodyForwardVector * BrakingForceOffset) };
	// DrawDebugSphere(GetWorld(), BrakingForceStart, 1.0f, 6, FColor::Magenta);
}

