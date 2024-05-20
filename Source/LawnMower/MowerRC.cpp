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

	DrawForceAndWheelRayCasts(ForceRayCasts, WheelRayCasts);

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


void AMowerRC::SendForceRayCasts(AllForceRayCasts& SentForceRayCasts, const LocalStarts& SentForceRayCastStarts)
{
	if (RayCastHit(SentForceRayCasts.FR, SentForceRayCastStarts.FR)) ApplyForceOnRayCast(SentForceRayCasts.FR);
	if (RayCastHit(SentForceRayCasts.FL, SentForceRayCastStarts.FL)) ApplyForceOnRayCast(SentForceRayCasts.FL);
	if (RayCastHit(SentForceRayCasts.BR, SentForceRayCastStarts.BR)) ApplyForceOnRayCast(SentForceRayCasts.BR);
	if (RayCastHit(SentForceRayCasts.BL, SentForceRayCastStarts.BL)) ApplyForceOnRayCast(SentForceRayCasts.BL);
}


bool AMowerRC::RayCastHit(RayCast& RayCast, const FVector& DefaultRayCastPosition)
{
	RayCast.RayCastStart = UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, DefaultRayCastPosition);
	RayCast.Hit.Reset();

	const FVector RayCastEnd{ RayCast.RayCastStart + (-PhysicsBodyUpVector * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(RayCast.Hit, RayCast.RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


void AMowerRC::ApplyForceOnRayCast(RayCast& RayCast)
{
	RayCast.CompressionRatio = (RayCastLength - RayCast.Hit.Distance) / RayCastLength;

	if (RayCast.CompressionRatio < DragForceCompressionRatioMinimum) RayCast.DragForce = MaxWheelDragForce;
	else RayCast.DragForce = MaxWheelDragForce / (RayCast.CompressionRatio * WheelCount);
	
	DragForces.Add(RayCast.DragForce);

	double Acceleration{ GravitationalAcceleration };
	Acceleration = GravitationalAcceleration * RayCast.CompressionRatio;

	const double Force{ Mass * Acceleration };
	const FVector RayCastForce{ RayCast.Hit.ImpactNormal * Force };

	PhysicsBody->AddForceAtLocation(RayCastForce, RayCast.RayCastStart);
}


void AMowerRC::SendWheelRayCasts(AllWheelRayCasts& SentWheelRayCasts, const LocalStarts& SentWheelRayCastStarts)
{
	ApplySuspensionOnWheel(FRWheel, SentWheelRayCasts.FR, SentWheelRayCastStarts.FR);
	ApplySuspensionOnWheel(FLWheel, SentWheelRayCasts.FL, SentWheelRayCastStarts.FL);
	ApplySuspensionOnWheel(BRWheel, SentWheelRayCasts.BR, SentWheelRayCastStarts.BR);
	ApplySuspensionOnWheel(BLWheel, SentWheelRayCasts.BL, SentWheelRayCastStarts.BL);
}


void AMowerRC::ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, WheelCast& WheelCast, const FVector& DefaultWheelPosition)
{
	WheelCast.WheelCastStart = UKismetMathLibrary::TransformLocation(PhysicsBodyTransform, DefaultWheelPosition);
	WheelCast.Hit.Reset();
	
	const FVector WheelEnd{ WheelCast.WheelCastStart + (-PhysicsBodyUpVector * RayCastLength) };

	double RayCastLengthDifference{};

	if (GetWorld()->LineTraceSingleByChannel(WheelCast.Hit, WheelCast.WheelCastStart, WheelEnd, ECC_GameTraceChannel1) )
		RayCastLengthDifference = RayCastLength - WheelCast.Hit.Distance;

	Wheel->SetWorldLocation(WheelCast.WheelCastStart + (PhysicsBodyUpVector * RayCastLengthDifference));
}


void AMowerRC::DrawForceAndWheelRayCasts(AllForceRayCasts& SentForceRayCasts, AllWheelRayCasts& SentWheelRayCasts)
{
	DrawRayCastData(SentForceRayCasts.FR.RayCastStart, SentForceRayCasts.FR.Hit);
	DrawRayCastData(SentForceRayCasts.FL.RayCastStart, SentForceRayCasts.FL.Hit);
	DrawRayCastData(SentForceRayCasts.BR.RayCastStart, SentForceRayCasts.BR.Hit);
	DrawRayCastData(SentForceRayCasts.BL.RayCastStart, SentForceRayCasts.BL.Hit);

	DrawRayCastData(SentWheelRayCasts.FR.WheelCastStart, SentWheelRayCasts.FR.Hit);
	DrawRayCastData(SentWheelRayCasts.FL.WheelCastStart, SentWheelRayCasts.FL.Hit);
	DrawRayCastData(SentWheelRayCasts.BR.WheelCastStart, SentWheelRayCasts.BR.Hit);
	DrawRayCastData(SentWheelRayCasts.BL.WheelCastStart, SentWheelRayCasts.BL.Hit);
}


void AMowerRC::DrawRayCastData(FVector RayCastStart, FHitResult Hit)
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

	// const FVector SurfaceCrossProduct{ FVector::CrossProduct(PhysicsBodyRightVector, Hit.ImpactNormal) };
	// DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + (SurfaceCrossProduct * 50.0), FColor::Cyan);
	// // DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + (Hit.ImpactNormal * 50.0), FColor::Turquoise);
}


void AMowerRC::ApplyDragForce()
{
	double TotalDragForce{};

	for (double Force : DragForces) TotalDragForce += Force;

	const double LinearDragForce{ TotalDragForce * LinearDragForceMultiplier };
	const double AngularDragForce{ TotalDragForce * AngularDragForceMultiplier };

	// UE_LOG(LogTemp, Warning, TEXT("Linear Drag: %f	Angular Drag: %f"), LinearDragForce, AngularDragForce);

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
