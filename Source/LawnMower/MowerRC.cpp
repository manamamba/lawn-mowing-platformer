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
	PhysicsBody->SetCollisionProfileName("PhysicsActor");

	CameraArm->SetRelativeLocation(FVector{ 0.0, 0.0, 10.0 });
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
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

	PhysicsBody->SetCenterOfMass(FVector{ 0.0, 0.0, -10.0 });
}


void AMowerRC::AddInputMappingContextToLocalPlayerSubsystem()
{
	APlayerController* PlayerController{};
	UEnhancedInputLocalPlayerSubsystem* Subsystem{};

	PlayerController = Cast<APlayerController>(Controller);
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (PlayerController && Subsystem) Subsystem->AddMappingContext(InputMappingContext, 0);
}


void AMowerRC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// FloatMower();
	// TrackMowerForceDirection(DeltaTime);

	SetPhysicsBodyTickData();
	SendForceRayCasts(ForceRayCasts, ForceRayCastStarts);
	SendWheelRayCasts(WheelRayCasts, WheelRayCastStarts);

	DrawRayCasts(ForceRayCasts);
	// DrawRayCasts(WheelRayCasts);

	ApplyDragForce();
}


void AMowerRC::FloatMower()
{
	PhysicsBody->AddForce(FVector::UpVector * AntiGravitationalForce);
}


void AMowerRC::TrackMowerForceDirection(float DeltaTime)
{
	const FVector MowerVelocityThisTick{ PhysicsBody->GetComponentVelocity() };
	const double Acceleration{ GetAcceleration(MowerVelocityThisTick, MowerVelocity, DeltaTime) };
	const double Force{ Mass * Acceleration };
	const FVector VelocityNormal{ MowerVelocity.Final.GetSafeNormal(1.0) };
	const FVector LineStart{ PhysicsBody->GetComponentLocation() };
	const FVector LineEnd{ LineStart + (-VelocityNormal * 50.0) };

	DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Blue);
}


double AMowerRC::GetAcceleration(const FVector& Vector, ChangeInVelocity& Velocity, float DeltaTime)
{
	Velocity.Final = Vector;
	Velocity.Initial = Velocity.LastTick;
	Velocity.LastTick = Velocity.Final;

	return double{ FVector::Dist(Velocity.Final, Velocity.Initial) / DeltaTime };
}


void AMowerRC::SetPhysicsBodyTickData()
{
	PhysicsBodyTransform = PhysicsBody->GetComponentTransform();
	PhysicsBodyUpVector = PhysicsBody->GetUpVector();
	PhysicsBodyForwardVector = PhysicsBody->GetForwardVector();
	PhysicsBodyRightVector = PhysicsBody->GetRightVector();
}


void AMowerRC::SendForceRayCasts(RayCastGroup& SentRayCasts, const LocalStarts& SentRayCastStarts)
{
	if (RayCastHit(SentRayCasts.FR, SentRayCastStarts.FR)) ApplyForceOnRayCast(SentRayCasts.FR);
	if (RayCastHit(SentRayCasts.FL, SentRayCastStarts.FL)) ApplyForceOnRayCast(SentRayCasts.FL);
	if (RayCastHit(SentRayCasts.BR, SentRayCastStarts.BR)) ApplyForceOnRayCast(SentRayCasts.BR);
	if (RayCastHit(SentRayCasts.BL, SentRayCastStarts.BL)) ApplyForceOnRayCast(SentRayCasts.BL);
}


bool AMowerRC::RayCastHit(RayCast& RayCast, const FVector& LocalPosition)
{
	RayCast.Hit.Reset();
	
	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, LocalPosition) };
	const FVector RayCastEnd{ RayCastStart + (-PhysicsBodyUpVector * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(RayCast.Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


void AMowerRC::ApplyForceOnRayCast(RayCast& RayCast)
{
	RayCast.CompressionRatio = (RayCastLength - RayCast.Hit.Distance) / RayCastLength;

	if (RayCast.CompressionRatio < DragForceCompressionRatioMinimum) RayCast.DragForce = MaxWheelDragForce;
	else RayCast.DragForce = MaxWheelDragForce / (RayCast.CompressionRatio * WheelCount);
	
	DragForces.Add(RayCast.DragForce);

	const double Force{ Mass * GravitationalAcceleration * RayCast.CompressionRatio };
	const FVector RayCastForce{ RayCast.Hit.ImpactNormal * Force };

	PhysicsBody->AddForceAtLocation(RayCastForce, RayCast.Hit.TraceStart);
}


void AMowerRC::SendWheelRayCasts(RayCastGroup& SentWheelRayCasts, const LocalStarts& SentWheelRayCastStarts)
{
	ApplySuspensionOnWheel(FRWheel, SentWheelRayCasts.FR, SentWheelRayCastStarts.FR);
	ApplySuspensionOnWheel(FLWheel, SentWheelRayCasts.FL, SentWheelRayCastStarts.FL);
	ApplySuspensionOnWheel(BRWheel, SentWheelRayCasts.BR, SentWheelRayCastStarts.BR);
	ApplySuspensionOnWheel(BLWheel, SentWheelRayCasts.BL, SentWheelRayCastStarts.BL);
}


void AMowerRC::ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, RayCast& WheelCast, const FVector& DefaultWheelPosition)
{
	WheelCast.Hit.Reset();
	
	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, DefaultWheelPosition) };
	const FVector WheelEnd{ WheelStart + (-PhysicsBodyUpVector * RayCastLength) };

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(WheelCast.Hit, WheelStart, WheelEnd, ECC_GameTraceChannel1) };
	double RayCastLengthDifference{};

	if (Grounded) RayCastLengthDifference = RayCastLength - WheelCast.Hit.Distance;

	Wheel->SetWorldLocation(WheelStart + (PhysicsBodyUpVector * RayCastLengthDifference));
}


void AMowerRC::DrawRayCasts(RayCastGroup& RayCasts)
{
	DrawRayCast(RayCasts.FR.Hit);
	DrawRayCast(RayCasts.FL.Hit);
	DrawRayCast(RayCasts.BR.Hit);
	DrawRayCast(RayCasts.BL.Hit);
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
	const float InputVector{ Value.Get<float>() };
	const double Acceleration{ 5000.0 };

	const FVector ForwardVector{ PhysicsBody->GetForwardVector() };
	const FVector Force{ ForwardVector * (Acceleration * InputVector) };

	PhysicsBody->AddForce(Force, NAME_None, true);
}
