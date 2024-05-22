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
	PhysicsBody->SetMassOverrideInKg(NAME_None, PhysicsBodyMass);
	PhysicsBody->SetCenterOfMass(PhysicsBodyCenterOfMass);
}


void AMowerRC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyAccelerationInput(ForceRayCasts, DeltaTime);
	ApplyBrakeFriction(DeltaTime);



	ApplyAccelerationForce();

	UpdatePhysicsBodyPositionalData();
	UpdatePhysicsBodyForceData(DeltaTime);

	SendForceRayCasts(ForceRayCasts, ForceRayCastOrigins);
	SendWheelRayCasts(WheelRayCasts, WheelRayCastOrigins);

	// DrawRayCasts(ForceRayCasts);
	// DrawRayCasts(WheelRayCasts);

	ApplyDragForces();
}


void AMowerRC::FloatMower() { PhysicsBody->AddForce(FVector::UpVector * AntiGravitationalForce); }


void AMowerRC::ApplyAccelerationInput(RayCastGroup& RayCastGroup, float DeltaTime)
{
	FVector AccelerationSurfaceImpact{ GetAccelerationSurfaceImpact(RayCastGroup) };
	FVector AccelerationSurfaceNormal{ GetAccelerationSurfaceNormal(RayCastGroup) };

	// AcceleratingDirection



	if (GroundedWheels) AccelerationRatio += AcceleratingDirection * DeltaTime;

	if (AccelerationRatio > AccelerationRatioMaximum) AccelerationRatio = AccelerationRatioMaximum;
	if (AccelerationRatio < -AccelerationRatioMaximum) AccelerationRatio = -AccelerationRatioMaximum;

	AccelerationForce = AccelerationForceMaximum * AccelerationRatio;

	if (AccelerationForce < 0.0) AccelerationSurfaceNormal = -AccelerationSurfaceNormal;

	PhysicsBody->AddForceAtLocation(AccelerationSurfaceNormal * abs(AccelerationForce), AccelerationSurfaceImpact);

	if (!AcceleratingDirection || !GroundedWheels)
	{
		if (AccelerationRatio < 0) AccelerationRatio += DeltaTime * AccelerationDecayRate;
		if (AccelerationRatio > 0) AccelerationRatio -= DeltaTime * AccelerationDecayRate;
		if (AccelerationRatio < 0.1 && AccelerationRatio > -0.1) AccelerationRatio = 0.0;
	}

	UE_LOG(LogTemp, Warning, TEXT("AccelerationForce %f"), AccelerationForce);
	UE_LOG(LogTemp, Warning, TEXT("AccelerationRatio %f"), AccelerationRatio);
	// DrawDebugSphere(GetWorld(), AccelerationSurfaceImpact, 1.0f, 6, FColor::Orange);
	// DrawDebugSphere(GetWorld(), AccelerationSurfaceImpact + (AccelerationSurfaceNormal * 50.0), 1.0f, 6, FColor::Orange);
	// DrawDebugLine(GetWorld(), AccelerationSurfaceImpact, AccelerationSurfaceImpact + (AccelerationSurfaceNormal * 50.0), FColor::Orange);

	AcceleratingDirection = 0.0;
}


FVector AMowerRC::GetAccelerationSurfaceImpact(const RayCastGroup& RayCastGroup)
{
	double AccelerationSurface{};

	if (RayCastGroup.FR.bBlockingHit) AccelerationSurface += RayCastGroup.FR.ImpactPoint.Z;
	if (RayCastGroup.FL.bBlockingHit) AccelerationSurface += RayCastGroup.FL.ImpactPoint.Z;
	if (RayCastGroup.BR.bBlockingHit) AccelerationSurface += RayCastGroup.BR.ImpactPoint.Z;
	if (RayCastGroup.BL.bBlockingHit) AccelerationSurface += RayCastGroup.BL.ImpactPoint.Z;

	FVector AccelerationSurfaceImpact{ PhysicsBodyLocation };

	if (GroundedWheels) AccelerationSurfaceImpact.Z = AccelerationSurface / GroundedWheels;

	return AccelerationSurfaceImpact;
}


FVector AMowerRC::GetAccelerationSurfaceNormal(const RayCastGroup& RayCastGroup)
{
	FVector AccelerationSurfaceNormal{};

	if (RayCastGroup.FR.bBlockingHit) AccelerationSurfaceNormal += RayCastGroup.FR.ImpactNormal;
	if (RayCastGroup.FL.bBlockingHit) AccelerationSurfaceNormal += RayCastGroup.FL.ImpactNormal;
	if (RayCastGroup.BR.bBlockingHit) AccelerationSurfaceNormal += RayCastGroup.BR.ImpactNormal;
	if (RayCastGroup.BL.bBlockingHit) AccelerationSurfaceNormal += RayCastGroup.BL.ImpactNormal;

	if (GroundedWheels) return FVector::CrossProduct(PhysicsBodyRightVector, AccelerationSurfaceNormal / GroundedWheels);
	else return AccelerationSurfaceNormal;
}


void AMowerRC::ApplyBrakeFriction(float DeltaTime)
{
	if (GroundedWheels) AccelerationRatio -= AcceleratingDirection * DeltaTime;

}


void AMowerRC::ApplyAccelerationForce()
{



}





void AMowerRC::UpdatePhysicsBodyPositionalData()
{
	PhysicsBodyTransform = PhysicsBody->GetComponentTransform();
	PhysicsBodyLocation = PhysicsBody->GetComponentLocation();
	PhysicsBodyUpVector = PhysicsBody->GetUpVector();
	PhysicsBodyForwardVector = PhysicsBody->GetForwardVector();
	PhysicsBodyRightVector = PhysicsBody->GetRightVector();
}


void AMowerRC::UpdatePhysicsBodyForceData(float DeltaTime)
{
	PhysicsBodyVelocity = PhysicsBody->GetComponentVelocity();
	PhysicsBodyFinalVelocity = PhysicsBodyVelocity;
	PhysicsBodyInitiallVelocity = PhysicsBodyLastTicklVelocity;
	PhysicsBodyLastTicklVelocity = PhysicsBodyFinalVelocity;
	PhysicsBodyVelocityNormal = PhysicsBodyFinalVelocity.GetSafeNormal(1.0);
	PhysicsBodyChangeInVelocity = FVector::Dist(PhysicsBodyFinalVelocity, PhysicsBodyInitiallVelocity);
	PhysicsBodyAcceleration = PhysicsBodyChangeInVelocity / DeltaTime;
	PhysicsBodyForce = PhysicsBodyMass * PhysicsBodyAcceleration;
}


void AMowerRC::SendForceRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins)
{
	GroundedWheels = 0;

	if (RayCastHit(RayCastGroup.FR, LocalOrigins.FR)) AddForceOnRayCastHit(RayCastGroup.FR);
	if (RayCastHit(RayCastGroup.FL, LocalOrigins.FL)) AddForceOnRayCastHit(RayCastGroup.FL);
	if (RayCastHit(RayCastGroup.BR, LocalOrigins.BR)) AddForceOnRayCastHit(RayCastGroup.BR);
	if (RayCastHit(RayCastGroup.BL, LocalOrigins.BL)) AddForceOnRayCastHit(RayCastGroup.BL);
}


bool AMowerRC::RayCastHit(FHitResult& RayCast, const FVector& LocalOrigin)
{
	RayCast.Reset();
	
	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, LocalOrigin) };
	const FVector RayCastEnd{ RayCastStart + (-PhysicsBodyUpVector * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(RayCast, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


void AMowerRC::AddForceOnRayCastHit(FHitResult& RayCast)
{
	const double CompressionRatio{ 1.0 - RayCast.Time };

	AddDragForceOnRayCastHit(CompressionRatio);

	const double Force{ PhysicsBodyMass * GravitationalAcceleration * CompressionRatio };

	PhysicsBody->AddForceAtLocation(RayCast.ImpactNormal * Force, RayCast.TraceStart);
}


void AMowerRC::AddDragForceOnRayCastHit(double CompressionRatio)
{
	++GroundedWheels;
	
	double DragForce{};

	if (CompressionRatio < DragForceCompressionRatioMinimum) DragForce = MaxWheelDragForce;
	else DragForce = MaxWheelDragForce / (CompressionRatio * WheelCount);
	
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
	
	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, LocalOrigin) };
	const FVector WheelEnd{ WheelStart + (-PhysicsBodyUpVector * RayCastLength) };
	
	double RayCastLengthDifference{};

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(RayCast, WheelStart, WheelEnd, ECC_GameTraceChannel1) };
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


void AMowerRC::DrawRayCast(FHitResult& RayCast)
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


void AMowerRC::ApplyDragForces()
{
	AngularDragForces.Add(abs(AccelerationForce * GroundedWheels) * AngularDragForceMultiplier);
	
	double TotalLinearDragForce{ 0.01 };
	double TotalAngularDragForce{};

	for (double Force : LinearDragForces) TotalLinearDragForce += Force;
	for (double Force : AngularDragForces) TotalAngularDragForce += Force;

	PhysicsBody->SetLinearDamping(TotalLinearDragForce);
	PhysicsBody->SetAngularDamping(TotalAngularDragForce);

	LinearDragForces.Reset();
	AngularDragForces.Reset();

	UE_LOG(LogTemp, Warning, TEXT("TotalLinearDragForce %f"), TotalLinearDragForce);
	UE_LOG(LogTemp, Warning, TEXT("TotalAngularDragForce %f"), TotalAngularDragForce);
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
	BrakeFriction = Value.Get<float>();
}


void AMowerRC::Steer(const FInputActionValue& Value)
{

}
