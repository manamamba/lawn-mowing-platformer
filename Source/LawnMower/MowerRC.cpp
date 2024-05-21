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

	CameraArm->SetRelativeLocation(FVector{ 0.0, 0.0, 10.0 });
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
	CameraArm->SetUsingAbsoluteRotation(true);
	CameraArm->TargetArmLength = 200.0f;

	SetMeshComponentCollisionAndDefaultLocation(Body, BodyPosition);
	SetMeshComponentCollisionAndDefaultLocation(Handle, HandlePosition);
	SetMeshComponentCollisionAndDefaultLocation(FRWheel, FRWheelPosition);
	SetMeshComponentCollisionAndDefaultLocation(FLWheel, FLWheelPosition);
	SetMeshComponentCollisionAndDefaultLocation(BRWheel, BRWheelPosition);
	SetMeshComponentCollisionAndDefaultLocation(BLWheel, BLWheelPosition);
}


void AMowerRC::SetMeshComponentCollisionAndDefaultLocation(UStaticMeshComponent* Mesh, const FVector& Location)
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
	PhysicsBody->SetCenterOfMass(FVector{ 0.0, 0.0, -10.0 });
}


void AMowerRC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// accelerate function
	
	UpdatePhysicsBodyPositionalData();
	UpdatePhysicsBodyForceData(DeltaTime);

	SendForceRayCasts(ForceRayCasts, ForceRayCastOrigins);
	SendWheelRayCasts(WheelRayCasts, WheelRayCastOrigins);

	// DrawRayCasts(ForceRayCasts);
	// DrawRayCasts(WheelRayCasts);

	ApplyDragForce();
}


void AMowerRC::UpdatePhysicsBodyPositionalData()
{
	PhysicsBodyTransform = PhysicsBody->GetComponentTransform();
	PhysicsBodyLocation = PhysicsBody->GetComponentLocation();
	PhysicsBodyUpVector = PhysicsBody->GetUpVector();
	PhysicsBodyForwardVector = PhysicsBody->GetForwardVector();
	PhysicsBodyRightVector = PhysicsBody->GetRightVector();
	PhysicsBodyVelocity = PhysicsBody->GetComponentVelocity();
}

void AMowerRC::UpdatePhysicsBodyForceData(float DeltaTime)
{
	PhysicsBodyFinalVelocity = PhysicsBodyVelocity;
	PhysicsBodyInitiallVelocity = PhysicsBodyLastTicklVelocity;
	PhysicsBodyLastTicklVelocity = PhysicsBodyFinalVelocity;
	PhysicsBodyVelocityNormal = PhysicsBodyFinalVelocity.GetSafeNormal(1.0);
	PhysicsBodyChangeInVelocity = FVector::Dist(PhysicsBodyFinalVelocity, PhysicsBodyInitiallVelocity);
	PhysicsBodyAcceleration = PhysicsBodyChangeInVelocity / DeltaTime;
	PhysicsBodyForce = PhysicsBodyMass * PhysicsBodyAcceleration;
}


void AMowerRC::FloatMower() { PhysicsBody->AddForce(FVector::UpVector * AntiGravitationalForce); }


void AMowerRC::SendForceRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins)
{
	if (RayCastHit(RayCastGroup.FR, LocalOrigins.FR)) AddForceOnRayCastHit(RayCastGroup.FR);
	if (RayCastHit(RayCastGroup.FL, LocalOrigins.FL)) AddForceOnRayCastHit(RayCastGroup.FL);
	if (RayCastHit(RayCastGroup.BR, LocalOrigins.BR)) AddForceOnRayCastHit(RayCastGroup.BR);
	if (RayCastHit(RayCastGroup.BL, LocalOrigins.BL)) AddForceOnRayCastHit(RayCastGroup.BL);
}


bool AMowerRC::RayCastHit(FHitResult& Hit, const FVector& LocalOrigin)
{
	Hit.Reset();
	
	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, LocalOrigin) };
	const FVector RayCastEnd{ RayCastStart + (-PhysicsBodyUpVector * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


void AMowerRC::AddForceOnRayCastHit(FHitResult& Hit)
{
	double CompressionRatio{ 1.0 - Hit.Time };

	AddDragOnRayCastHit(CompressionRatio);

	const double Force{ PhysicsBodyMass * GravitationalAcceleration * CompressionRatio };

	PhysicsBody->AddForceAtLocation(Hit.ImpactNormal * Force, Hit.TraceStart);
}


void AMowerRC::AddDragOnRayCastHit(double CompressionRatio)
{
	double DragForce{};

	if (CompressionRatio < DragForceCompressionRatioMinimum) DragForce = MaxWheelDragForce;
	else DragForce = MaxWheelDragForce / (CompressionRatio * WheelCount);
	
	DragForces.Add(DragForce);
}


void AMowerRC::SendWheelRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins)
{
	ApplySuspensionOnWheel(FRWheel, RayCastGroup.FR, LocalOrigins.FR);
	ApplySuspensionOnWheel(FLWheel, RayCastGroup.FL, LocalOrigins.FL);
	ApplySuspensionOnWheel(BRWheel, RayCastGroup.BR, LocalOrigins.BR);
	ApplySuspensionOnWheel(BLWheel, RayCastGroup.BL, LocalOrigins.BL);
}


void AMowerRC::ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, FHitResult& Hit, const FVector& LocalOrigin)
{
	Hit.Reset();
	
	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, LocalOrigin) };
	const FVector WheelEnd{ WheelStart + (-PhysicsBodyUpVector * RayCastLength) };
	
	double RayCastLengthDifference{};

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(Hit, WheelStart, WheelEnd, ECC_GameTraceChannel1) };
	if (Grounded) RayCastLengthDifference = RayCastLength - Hit.Distance;

	Wheel->SetWorldLocation(WheelStart + (PhysicsBodyUpVector * RayCastLengthDifference));
}


void AMowerRC::DrawRayCasts(RayCastGroup& RayCasts)
{
	DrawRayCast(RayCasts.FR);
	DrawRayCast(RayCasts.FL);
	DrawRayCast(RayCasts.BR);
	DrawRayCast(RayCasts.BL);
}


void AMowerRC::DrawRayCast(FHitResult& Hit)
{
	if (!Hit.bBlockingHit)
	{
		DrawDebugLine(GetWorld(), Hit.TraceStart, Hit.TraceEnd, FColor::Red);
		DrawDebugSphere(GetWorld(), Hit.TraceEnd, 1.0, 6, FColor::Red);

		return;
	}

	const FVector SurfaceCrossProduct{ FVector::CrossProduct(PhysicsBodyRightVector, Hit.ImpactNormal) };

	DrawDebugLine(GetWorld(), Hit.TraceStart, Hit.ImpactPoint, FColor::Green);
	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 1.0, 6, FColor::Green);

	DrawDebugLine(GetWorld(), Hit.TraceStart, Hit.ImpactPoint + (Hit.ImpactNormal * 25.0), FColor::Turquoise);
	DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + (SurfaceCrossProduct * 25.0), FColor::Cyan);
	
}


void AMowerRC::ApplyDragForce()
{
	double TotalDragForce{};

	for (double Force : DragForces) TotalDragForce += Force;

	const double LinearDragForce{ TotalDragForce * LinearDragForceMultiplier };
	const double AngularDragForce{ TotalDragForce * AngularDragForceMultiplier };

	PhysicsBody->SetLinearDamping(LinearDragForce);
	PhysicsBody->SetAngularDamping(AngularDragForce);

	DragForces.Reset();
}


void AMowerRC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		EnhancedInputComponent->BindAction(MoveCameraInputAction, ETriggerEvent::Triggered, this, &AMowerRC::MoveCamera);
		EnhancedInputComponent->BindAction(AccelerateInputAction, ETriggerEvent::Triggered, this, &AMowerRC::Accelerate);
	}
}


void AMowerRC::MoveCamera(const FInputActionValue& Value)
{
	const FVector2D InputVector{ Value.Get<FVector2D>() };
	
	FRotator ArmPosition{ CameraArm->GetRelativeRotation() };

	ArmPosition.Yaw += InputVector.X;
	ArmPosition.Pitch += InputVector.Y;

	if (ArmPosition.Pitch > MaxArmPitch) ArmPosition.Pitch = MaxArmPitch;
	if (ArmPosition.Pitch < MinArmPitch) ArmPosition.Pitch = MinArmPitch;

	CameraArm->SetRelativeRotation(ArmPosition);
}


void AMowerRC::Accelerate(const FInputActionValue& Value)
{
	AcceleratingDirection =  Value.Get<float>();

	// const double Acceleration{ 5000.0 };
	// const FVector ForwardVector{ PhysicsBody->GetForwardVector() };
	// const FVector Force{ ForwardVector * (Acceleration * AcceleratingDirection) };
	// PhysicsBody->AddForce(Force, NAME_None, true);
	//const FVector SurfaceCrossProduct{ FVector::CrossProduct(PhysicsBodyRightVector, Hit.ImpactNormal) };
}
