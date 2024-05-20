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

	if (RayCastHit(FRRayCast, FRRayCastPosition)) ApplyForceOnRayCast(FRRayCast);
	if (RayCastHit(FRRayCast, FLRayCastPosition)) ApplyForceOnRayCast(FRRayCast);
	if (RayCastHit(FRRayCast, BRRayCastPosition)) ApplyForceOnRayCast(FRRayCast);
	if (RayCastHit(FRRayCast, BLRayCastPosition)) ApplyForceOnRayCast(FRRayCast);

	ApplySuspensionOnWheel(FRWheel, FRWheelPosition);
	ApplySuspensionOnWheel(FLWheel, FLWheelPosition);
	ApplySuspensionOnWheel(BRWheel, BRWheelPosition);
	ApplySuspensionOnWheel(BLWheel, BLWheelPosition);

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









bool AMowerRC::RayCastHit(RayCast& RayCast, const FVector& DefaultRayCastPosition)
{
	const FTransform ComponentTransform{ PhysicsBody->GetComponentTransform() };
	const FVector ComponentUpVector{ PhysicsBody->GetUpVector() };

	RayCast.RayCastStart = UKismetMathLibrary::TransformLocation(ComponentTransform, DefaultRayCastPosition);
	RayCast.Hit.Reset();

	const FVector RayCastEnd{ RayCast.RayCastStart + (-ComponentUpVector * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(RayCast.Hit, RayCast.RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


void AMowerRC::ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, const FVector& DefaultWheelPosition)
{
	const FTransform ComponentTransform{ PhysicsBody->GetComponentTransform() };
	const FVector ComponentUpVector{ PhysicsBody->GetUpVector() };

	FHitResult Hit{};

	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(ComponentTransform, DefaultWheelPosition) };
	const FVector WheelEnd{ WheelStart + (-ComponentUpVector * RayCastLength) };

	double RayCastLengthDifference{};

	if (GetWorld()->LineTraceSingleByChannel(Hit, WheelStart, WheelEnd, ECC_GameTraceChannel1) )
		RayCastLengthDifference = RayCastLength - Hit.Distance;

	Wheel->SetWorldLocation(WheelStart + (ComponentUpVector * RayCastLengthDifference));
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


























void AMowerRC::RayCastAtDefaultPosition(UPrimitiveComponent* Component, const FVector& DefaultRayCastPosition, UStaticMeshComponent* Mesh, const FVector& DefaultMeshPosition)
{
	if (!Component) return;
	
	FHitResult Hit{};

	const FTransform ComponentTransform{ Component->GetComponentTransform() };
	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(ComponentTransform, DefaultRayCastPosition) };
	const FVector ComponentUpVector{ Component->GetUpVector() };
	const FVector RayCastEnd{ RayCastStart + (-ComponentUpVector * RayCastLength) };

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1) };

	double RayCastLengthDifference{};
	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(ComponentTransform, DefaultMeshPosition) };
	
	if (Grounded) RayCastLengthDifference = RayCastLength - Hit.Distance;
	
	const double CompressionRatio{ RayCastLengthDifference / RayCastLength }; 
	const FVector SurfaceIP{ Hit.ImpactPoint };
	const FVector SurfaceIN{ Hit.ImpactNormal };

	Mesh->SetWorldLocation(WheelStart + (ComponentUpVector * RayCastLengthDifference));


	if (!Grounded)
	{
		DrawDebugLine(GetWorld(), Hit.TraceStart, Hit.TraceEnd, FColor::Red);
		DrawDebugSphere(GetWorld(), Hit.TraceEnd, 1.0, 6, FColor::Red);

		return;
	}
	else
	{
		const FVector SurfaceCrossProduct{ FVector::CrossProduct(Component->GetRightVector(), SurfaceIN) };

		DrawDebugLine(GetWorld(), RayCastStart, SurfaceIP, FColor::Green);
		DrawDebugSphere(GetWorld(), SurfaceIP, 1.0, 6, FColor::Green);
		DrawDebugLine(GetWorld(), SurfaceIP, SurfaceIP + (SurfaceIN * 50.0), FColor::Turquoise);
		DrawDebugLine(GetWorld(), SurfaceIP, SurfaceIP + (SurfaceCrossProduct * 50.0), FColor::Cyan);

		++GroundedWheels;
	}


	if (CompressionRatio < DragCompression) DragCompression = CompressionRatio;
	if (DragCompression < DragCompressionMinimum) DragCompression = DragCompressionMinimum;


	double Acceleration{ GravitationalAcceleration };
	Acceleration = GravitationalAcceleration * CompressionRatio;

	const double Force{ Mass * Acceleration };
	const FVector RayCastForce{ SurfaceIN * Force };

	Component->AddForceAtLocation(RayCastForce, RayCastStart);
}


void AMowerRC::ApplyDragToGroundedMower()
{
	if (DragCompression == 1.0)
	{
		PhysicsBody->SetLinearDamping(0.01);
		PhysicsBody->SetAngularDamping(0.0);

		return;
	}
	
	double WheelDrag{ GroundedWheels / WheelCount };

	PhysicsBody->SetLinearDamping(MaxWheelDrag / DragCompression * WheelDrag);
	PhysicsBody->SetAngularDamping(MaxWheelDrag / DragCompression * WheelDrag);

	UE_LOG(LogTemp, Warning, TEXT("Drag: %f"), MaxWheelDrag / DragCompression * WheelDrag);

	DragCompression = 1.0;
	GroundedWheels = 0;
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
