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

	CameraArm->SetRelativeRotation(CameraArmRotation);
	CameraArm->TargetArmLength = 200.0f;
	CameraArm->ProbeSize = 5.0f;
	CameraArm->bInheritPitch = false;
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
	SetPhysicsBodyMassProperties();
}


void AMowerRC::AddInputMappingContextToLocalPlayerSubsystem()
{
	APlayerController* PlayerController{};
	UEnhancedInputLocalPlayerSubsystem* Subsystem{};

	PlayerController = Cast<APlayerController>(Controller);
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (PlayerController && Subsystem) Subsystem->AddMappingContext(InputMappingContext, 0);
}


void AMowerRC::SetPhysicsBodyMassProperties()
{
	const FVector PhysicsBodyCenterOfMass{ 0.0, 0.0, -PhysicsBodyMass / 2.0 };
	
	PhysicsBody->SetMassOverrideInKg(NAME_None, PhysicsBodyMass);
	PhysicsBody->SetCenterOfMass(PhysicsBodyCenterOfMass);
}


void AMowerRC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAcceleration(ForceRayCasts, DeltaTime);
	ApplyAcceleration();

	ResetDragForces();

	UpdatePhysicsBodyPositionData();

	SendForceRayCasts(ForceRayCasts, ForceRayCastOrigins);

	SendWheelRayCasts(WheelRayCasts, WheelRayCastOrigins);

	DrawRayCasts(ForceRayCasts);
	DrawRayCasts(WheelRayCasts);
	DrawAcceleration();

	AddAngularDragForces();
	ApplyDragForces();
}


void AMowerRC::FloatMower() { PhysicsBody->AddForce(FVector::UpVector * AntiGravitationalForce); }


void AMowerRC::UpdateAcceleration(RayCastGroup& RayCastGroup, float DeltaTime)
{
	const float AccelerationForceMaximum{ 10000.0f };
	const float AccelerationRatioMaximum{ 3.0f };
	const float AcceleratingDecayRate{ 0.5f };
	
	UpdateAccelerationSurfaceImpact(RayCastGroup);
	UpdateAccelerationSurfaceNormal(RayCastGroup);

	if (WheelsGrounded && !Braking) AccelerationRatio += AcceleratingDirection * DeltaTime;

	if ((WheelsGrounded && Braking) && AccelerationRatio < 0.0f) AccelerationRatio += Braking * DeltaTime;
	if ((WheelsGrounded && Braking) && AccelerationRatio > 0.0f) AccelerationRatio += -Braking * DeltaTime;

	if (AccelerationRatio > AccelerationRatioMaximum) AccelerationRatio = AccelerationRatioMaximum;
	if (AccelerationRatio < -AccelerationRatioMaximum) AccelerationRatio = -AccelerationRatioMaximum;

	AccelerationForce = AccelerationForceMaximum * TotalLinearDragForce * AccelerationRatio;

	if (AccelerationForce < 0.0f) AccelerationSurfaceNormal = -AccelerationSurfaceNormal;

	DecayAcceleration(AcceleratingDecayRate * DeltaTime);
}


void AMowerRC::UpdateAccelerationSurfaceImpact(const RayCastGroup& RayCastGroup)
{
	double SurfaceAverage{};

	if (RayCastGroup.FR.bBlockingHit) SurfaceAverage += RayCastGroup.FR.ImpactPoint.Z;
	if (RayCastGroup.FL.bBlockingHit) SurfaceAverage += RayCastGroup.FL.ImpactPoint.Z;
	if (RayCastGroup.BR.bBlockingHit) SurfaceAverage += RayCastGroup.BR.ImpactPoint.Z;
	if (RayCastGroup.BL.bBlockingHit) SurfaceAverage += RayCastGroup.BL.ImpactPoint.Z;

	AccelerationSurfaceImpact = PhysicsBodyLocation;

	if (WheelsGrounded) AccelerationSurfaceImpact.Z = SurfaceAverage / WheelsGrounded;
}


void AMowerRC::UpdateAccelerationSurfaceNormal(const RayCastGroup& RayCastGroup)
{
	FVector NormalAverage{};
	
	if (RayCastGroup.FR.bBlockingHit) NormalAverage += RayCastGroup.FR.ImpactNormal;
	if (RayCastGroup.FL.bBlockingHit) NormalAverage += RayCastGroup.FL.ImpactNormal;
	if (RayCastGroup.BR.bBlockingHit) NormalAverage += RayCastGroup.BR.ImpactNormal;
	if (RayCastGroup.BL.bBlockingHit) NormalAverage += RayCastGroup.BL.ImpactNormal;
	
	if (WheelsGrounded) NormalAverage = FVector::CrossProduct(PhysicsBodyRightVector, NormalAverage / WheelsGrounded);

	AccelerationSurfaceNormal = NormalAverage;
}


void AMowerRC::DecayAcceleration(float DecayRate)
{
	if (!AcceleratingDirection || (Braking && WheelsGrounded))
	{
		if (AccelerationRatio < 0.0f) AccelerationRatio += DecayRate;
		if (AccelerationRatio > 0.0f) AccelerationRatio -= DecayRate;
		if (AccelerationRatio < 0.1f && AccelerationRatio > -0.1f) AccelerationRatio = 0.0f;
	}

	AcceleratingDirection = 0.0f;
	Braking = 0.0f;

	UE_LOG(LogTemp, Warning, TEXT(" "));
	UE_LOG(LogTemp, Warning, TEXT("AccelerationForce %f"), AccelerationForce);
	UE_LOG(LogTemp, Warning, TEXT("AccelerationRatio %f"), AccelerationRatio);
}


void AMowerRC::ApplyAcceleration()
{
	PhysicsBody->AddForceAtLocation(AccelerationSurfaceNormal * abs(AccelerationForce), AccelerationSurfaceImpact);
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
	PhysicsBodyTransform = PhysicsBody->GetComponentTransform();
	PhysicsBodyLocation = PhysicsBody->GetComponentLocation();
	PhysicsBodyUpVector = PhysicsBody->GetUpVector();
	PhysicsBodyForwardVector = PhysicsBody->GetForwardVector();
	PhysicsBodyRightVector = PhysicsBody->GetRightVector();
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
	
	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, LocalOrigin) };
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
	const float CompressionRatioMinimum{ 0.25f };
	const float MaxDragForce{ 2.0f };
	const float WheelTotal{ 4.0f };
	float DragForce{};

	++WheelsGrounded;

	if (CompressionRatio < CompressionRatioMinimum) DragForce = MaxDragForce;
	else DragForce = MaxDragForce / (CompressionRatio * WheelTotal);
	
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

	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, LocalOrigin) };
	const FVector WheelEnd{ WheelStart + (-PhysicsBodyUpVector * RayCastLength) };
	const bool Grounded{ GetWorld()->LineTraceSingleByChannel(RayCast, WheelStart, WheelEnd, ECC_GameTraceChannel1) };

	if (Grounded) RayCastLengthDifference = RayCastLength - RayCast.Distance;

	Wheel->SetWorldLocation(WheelStart + (PhysicsBodyUpVector * RayCastLengthDifference));
}


// Wheel Animation Functions


void AMowerRC::DrawRayCasts(RayCastGroup& RayCasts)
{
	DrawRayCast(RayCasts.FR);
	DrawRayCast(RayCasts.FL);
	DrawRayCast(RayCasts.BR);
	DrawRayCast(RayCasts.BL);
}


void AMowerRC::DrawRayCast(const FHitResult& RayCast)
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


void AMowerRC::DrawAcceleration()
{
	const double DrawLineLength{ abs(AccelerationRatio * 15.0) };
	const FVector DrawStart{ AccelerationSurfaceImpact };
	const FVector DrawEnd{ AccelerationSurfaceImpact + (AccelerationSurfaceNormal * DrawLineLength) };

	DrawDebugSphere(GetWorld(), DrawStart, 1.0f, 6, FColor::Orange);
	DrawDebugLine(GetWorld(), DrawStart, DrawEnd, FColor::Orange);
	DrawDebugSphere(GetWorld(), DrawEnd, 1.0f, 6, FColor::Yellow);
}


void AMowerRC::AddAngularDragForces()
{
	const float AngularDragForceMultiplier{ 0.00002f };
	const float AngularAirTimeDrag{ 5.0f };

	AngularDragForces.Add(abs(AccelerationForce) * WheelsGrounded * AngularDragForceMultiplier);

	if (!WheelsGrounded) AngularDragForces.Add(AngularAirTimeDrag);
}


void AMowerRC::ApplyDragForces()
{
	for (float Force : LinearDragForces) TotalLinearDragForce += Force;
	for (float Force : AngularDragForces) TotalAngularDragForce += Force;

	PhysicsBody->SetLinearDamping(TotalLinearDragForce);
	PhysicsBody->SetAngularDamping(TotalAngularDragForce);

	UE_LOG(LogTemp, Warning, TEXT("TotalLinearDragForce %f"), TotalLinearDragForce);
	UE_LOG(LogTemp, Warning, TEXT("TotalAngularDragForce %f"), TotalAngularDragForce);

	if (TotalLinearDragForce == 0.01f) TotalLinearDragForce = 0.0f;
}


void AMowerRC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		EnhancedInputComponent->BindAction(MoveCameraInputAction, ETriggerEvent::Triggered, this, &AMowerRC::MoveCamera);
		EnhancedInputComponent->BindAction(AccelerateInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Accelerate);
		EnhancedInputComponent->BindAction(BrakeInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Brake);
		EnhancedInputComponent->BindAction(SteerInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Steer);
	}
}


void AMowerRC::MoveCamera(const FInputActionValue& Value)
{
	const FVector2D InputVector{ Value.Get<FVector2D>() };
	
	FRotator ArmPosition{ CameraArm->GetRelativeRotation() };

	ArmPosition.Yaw += InputVector.X;
	ArmPosition.Pitch += InputVector.Y;

	if (ArmPosition.Pitch > MaxCameraArmPitch) ArmPosition.Pitch = MaxCameraArmPitch;
	if (ArmPosition.Pitch < MinCameraArmPitch) ArmPosition.Pitch = MinCameraArmPitch;

	CameraArm->SetRelativeRotation(ArmPosition);
}


void AMowerRC::Accelerate(const FInputActionValue& Value)
{
	AcceleratingDirection = Value.Get<float>();
}


void AMowerRC::Brake(const FInputActionValue& Value)
{
	Braking = Value.Get<float>();
}


void AMowerRC::Steer(const FInputActionValue& Value)
{

}
