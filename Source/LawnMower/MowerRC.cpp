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
		EnhancedInputComponent->BindAction(DriftInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Drift);
	}
}


void AMowerRC::MoveCamera(const FInputActionValue& Value) { RotatingCameraDirection = Value.Get<FVector2D>(); }
void AMowerRC::ResetCamera(const FInputActionValue& Value) { bCameraReset = Value.Get<bool>(); }
void AMowerRC::Accelerate(const FInputActionValue& Value) { AcceleratingDirection = Value.Get<float>(); }
void AMowerRC::Brake(const FInputActionValue& Value) { Braking = Value.Get<float>(); }
void AMowerRC::Steer(const FInputActionValue& Value) { Steering = Value.Get<float>(); }
void AMowerRC::Drift(const FInputActionValue& Value) { Drifting = Value.Get<float>(); }


void AMowerRC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Float();

	UpdateTransforms();
	UpdateSpeed();
	UpdateCameraRotation();

	SendForceRayCasts(ForceRayCasts, ForceRayCastOrigins);

	UpdateAccelerationRatio(DeltaTime);
	UpdateDriftingRatio(DeltaTime);
	UpdateAcceleratingDirection();
	ApplyAccelerationForce();
	ApplySteeringTorque();
	ApplyDriftingForce();

	AddBrakingLinearDrag();
	AddAcceleratingAngularDrag();
	AddAirTimeAngularDrag();
	ApplyDrag();

	SendWheelSuspensionRayCasts(WheelRayCasts, WheelRayCastOrigins);

	// WheelAnimations

	LogData(DeltaTime);

	ResetDrag();
	ResetPlayerInputData();

	// DrawRayCastGroup(ForceRayCasts);
	// DrawRayCastGroup(WheelRayCasts);
	// DrawAcceleration();
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
	PhysicsBodySpeed = abs(FVector::Dist(LocationThisTick, LocationLastTick));
}


void AMowerRC::UpdateCameraRotation()
{
	if (bCameraReset) LocalCameraArmRotation = DefaultLocalCameraArmRotation;
	else LocalCameraArmRotation += FRotator{ RotatingCameraDirection.Y, RotatingCameraDirection.X, 0.0 };

	if (LocalCameraArmRotation.Pitch > MaxLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MaxLocalCameraArmPitch;
	if (LocalCameraArmRotation.Pitch < MinLocalCameraArmPitch) LocalCameraArmRotation.Pitch = MinLocalCameraArmPitch;

	const FRotator LocalCameraArmRotationThisTick{ UKismetMathLibrary::InverseTransformRotation(PhysicsBodyLocalTransform, WorldCameraArmRotation) };

	if (LocalCameraArmRotationThisTick != LocalCameraArmRotation) SetCameraArmWorldRotation();
}


void AMowerRC::SendForceRayCasts(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins)
{
	if (ForceRayCastHit(RayCastGroup.Fr, LocalOrigins.Fr)) AddForcesOnRayCastHit(RayCastGroup.Fr);
	if (ForceRayCastHit(RayCastGroup.Fl, LocalOrigins.Fl)) AddForcesOnRayCastHit(RayCastGroup.Fl);
	if (ForceRayCastHit(RayCastGroup.Br, LocalOrigins.Br)) AddForcesOnRayCastHit(RayCastGroup.Br);
	if (ForceRayCastHit(RayCastGroup.Bl, LocalOrigins.Bl)) AddForcesOnRayCastHit(RayCastGroup.Bl);
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
	if (AcceleratingDirection == 0.0f) DecayRatio(AccelerationRatio, AccelerationDecayRate, DeltaTime);

	if (!WheelsGrounded) return;

	if (AcceleratingDirection != 0.0f) AccelerationRatio += AcceleratingDirection * DeltaTime;

	LimitRatio(AccelerationRatio, AccelerationRatioMaximum);

	if (Braking)
	{
		if (AccelerationRatio > AccelerationRatioBrakingMinimum) AccelerationRatio += -Braking * AccelerationBrakingRate * DeltaTime;
		if (AccelerationRatio < -AccelerationRatioBrakingMinimum) AccelerationRatio += Braking * AccelerationBrakingRate * DeltaTime;
	}	
}


void AMowerRC::UpdateDriftingRatio(float DeltaTime)
{
	if (Steering < 0.0f && DriftingRatio < 0.0f) DriftingRatio = -DriftingRatio;
	if (Steering > 0.0f && DriftingRatio > 0.0f) DriftingRatio = -DriftingRatio;

	if (!Drifting || Braking) DecayRatio(DriftingRatio, DriftingForceDecayRate, DeltaTime);

	if (Drifting && Steering != 0.0f && AccelerationRatio != 0.0f && WheelsGrounded && !Braking)
	{
		DriftingRatio += -Steering * DriftingForceIncreaseRate * DeltaTime;
	}

	LimitRatio(DriftingRatio, DriftingRatioMaximum);
}


void AMowerRC::DecayRatio(float& Ratio, const float DecayRate, float DeltaTime)
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


void AMowerRC::ApplyDriftingForce()
{
	DriftingForcePosition =  AccelerationSurfaceImpact + (-PhysicsBodyForwardVector * DriftingForcePositionOffset);
	DriftingForce = abs(DriftingRatio) * DriftingForceMaximum * WheelsGrounded;

	if (DriftingRatio > 0.0f) PhysicsBody->AddForceAtLocation(PhysicsBodyRightVector * DriftingForce * AcceleratingDirection, DriftingForcePosition);
	if (DriftingRatio < 0.0f) PhysicsBody->AddForceAtLocation(-PhysicsBodyRightVector * DriftingForce * AcceleratingDirection, DriftingForcePosition);
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


void AMowerRC::SendWheelSuspensionRayCasts(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins)
{
	SetWheelSuspension(FrWheel, RayCastGroup.Fr, LocalOrigins.Fr);
	SetWheelSuspension(FlWheel, RayCastGroup.Fl, LocalOrigins.Fl);
	SetWheelSuspension(BrWheel, RayCastGroup.Br, LocalOrigins.Br);
	SetWheelSuspension(BlWheel, RayCastGroup.Bl, LocalOrigins.Bl);
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

	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT(" "));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("==================="));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("Speed               %f"), PhysicsBodySpeed);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("==================="));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationForce   %f"), AccelerationForce);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("SteeringForce       %f"), SteeringForce);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("DriftingForce       %f"), DriftingForce);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("==================="));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AccelerationRatio   %f"), AccelerationRatio);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("DriftingRatio       %f"), DriftingRatio);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("==================="));
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("LinearDrag          %f"), TotalLinearDrag);
	if (bTickReset) UE_LOG(LogTemp, Warning, TEXT("AngularDrag         %f"), TotalAngularDrag);
}


void AMowerRC::UpdateTickCount(float DeltaTime)
{
	TickCount += TickCountMultiplier * DeltaTime;

	if (TickCount > 1.0f) TickCount = 0.0f;

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
	Braking = 0.0f;
	Steering = 0.0f;
	Drifting = 0.0f;
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

	DrawDebugSphere(GetWorld(), DriftingForcePosition, 1.0f, 6, FColor::Magenta);
}

