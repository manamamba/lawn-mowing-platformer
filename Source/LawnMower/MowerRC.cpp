// Fill out your copyright notice in the Description page of Project Settings.


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

	// PhysicsBody->SetLinearDamping(2.0f);
	// PhysicsBody->SetAngularDamping(2.0f);

	PhysicsBody->SetUseCCD(true);
	PhysicsBody->SetCollisionProfileName("PhysicsActor");

	CameraArm->SetRelativeLocation(FVector{ 0.0, 0.0, 10.0 });
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
	CameraArm->TargetArmLength = 200.0f;

	SetMeshComponentCollisionAndDefaultLocation(Body, DefaultBodyPosition);
	SetMeshComponentCollisionAndDefaultLocation(Handle, DefaultHandlePosition);
	SetMeshComponentCollisionAndDefaultLocation(FRWheel, DefaultFRWheelPosition);
	SetMeshComponentCollisionAndDefaultLocation(FLWheel, DefaultFLWheelPosition);
	SetMeshComponentCollisionAndDefaultLocation(BRWheel, DefaultBRWheelPosition);
	SetMeshComponentCollisionAndDefaultLocation(BLWheel, DefaultBLWheelPosition);
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

	PhysicsBody->SetCenterOfMass(FVector{ 0.0, 0.0, -15.0 });

	AddInputMappingContextToLocalPlayerSubsystem();
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

	// FloatPhysicsEnabledComponent(PhysicsBody);
	// TrackPhysicsEnabledComponentAcceleration(PhysicsBody, PhysicsBodyVelocity, DeltaTime);

	RayCastAtDefaultPosition(PhysicsBody, FRRayCastDefaultPosition, FRWheel, DefaultFRWheelPosition);
	RayCastAtDefaultPosition(PhysicsBody, FLRayCastDefaultPosition, FLWheel, DefaultFLWheelPosition);
	RayCastAtDefaultPosition(PhysicsBody, BRRayCastDefaultPosition, BRWheel, DefaultBRWheelPosition);
	RayCastAtDefaultPosition(PhysicsBody, BLRayCastDefaultPosition, BLWheel, DefaultBLWheelPosition);

	ApplyDragToGroundedComponent(PhysicsBody);
}


void AMowerRC::FloatPhysicsEnabledComponent(UPrimitiveComponent* Component)
{
	if (!Component) return;
	
	const double Mass{ Component->GetMass() };
	const double GravitationalAcceleration{ 980.0 };
	const double Force{ Mass * GravitationalAcceleration };

	Component->AddForce(FVector::UpVector * Force);
}


void AMowerRC::TrackPhysicsEnabledComponentAcceleration(UPrimitiveComponent* Component, ChangeInVelocity& Velocity, float DeltaTime)
{
	if (!Component) return;

	const double Mass{ Component->GetMass() };
	const FVector ComponentVelocityThisTick{ Component->GetComponentVelocity() };
	const double ComponentAcceleration{ GetAcceleration(ComponentVelocityThisTick, Velocity, DeltaTime) };
	const double AntiVelocityForce{ Mass * ComponentAcceleration };
	const FVector VelocityDirection{ Velocity.Final.GetSafeNormal(1.0) };
	
	const FVector Start{ Component->GetComponentLocation() };
	const FVector End{ Start + (VelocityDirection * AntiVelocityForce / Mass) };
	DrawDebugLine(GetWorld(), Start, End, FColor::Blue);
	UE_LOG(LogTemp, Warning, TEXT("Acceleration      %f"), ComponentAcceleration);
	UE_LOG(LogTemp, Warning, TEXT("AntiVelocityForce %f"), AntiVelocityForce);
	UE_LOG(LogTemp, Warning, TEXT("VelocityDirection %s"), *VelocityDirection.ToString());
}


double AMowerRC::GetAcceleration(const FVector& Vector, ChangeInVelocity& Velocity, float DeltaTime)
{
	Velocity.Final = Vector;
	Velocity.Initial = Velocity.LastTick;
	Velocity.LastTick = Velocity.Final;

	return double{ FVector::Dist(Velocity.Final, Velocity.Initial) / DeltaTime };
}


void AMowerRC::RayCastAtDefaultPosition(UPrimitiveComponent* Component, const FVector& DefaultRayCastPosition, UStaticMeshComponent* Mesh, const FVector& DefaultMeshPosition)
{
	if (!Component) return;
	
	FHitResult Hit{};

	constexpr double RayCastLength{ 8.9 };
	const FTransform ComponentTransform{ Component->GetComponentTransform() };
	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(ComponentTransform, DefaultRayCastPosition) };
	const FVector ComponentUpVector{ Component->GetUpVector() };
	const FVector RayCastEnd{ RayCastStart + (-ComponentUpVector * RayCastLength) };

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1) };

	double RayCastLengthDifference{};
	double RayCastHitLength{ FVector::Dist(RayCastStart, Hit.ImpactPoint) };
	const FVector WheelStart{ UKismetMathLibrary::TransformLocation(ComponentTransform, DefaultMeshPosition) };
	
	if (Grounded) RayCastLengthDifference = RayCastLength - RayCastHitLength;
	
	const double CompressionRatio{ RayCastLengthDifference / RayCastLength }; 
	const FVector SurfaceImpactPoint{ Hit.ImpactPoint };
	const FVector SurfaceImpactNormal{ Hit.ImpactNormal };

	Mesh->SetWorldLocation(WheelStart + (ComponentUpVector * RayCastLengthDifference));


	if (!Grounded)
	{
		DrawDebugLine(GetWorld(), RayCastStart, RayCastEnd, FColor::Red);
		DrawDebugSphere(GetWorld(), RayCastEnd, 1.0, 6, FColor::Red);

		return;
	}
	else
	{
		DrawDebugLine(GetWorld(), RayCastStart, Hit.ImpactPoint, FColor::Green);
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 1.0, 6, FColor::Green);
		DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + (Hit.ImpactNormal * 50.0), FColor::Turquoise);

		++GroundedWheels;
	}
	

	const double WheelCount{ 4.0 };
	const double Mass{ Component->GetMass() };
	const double GravitationalAcceleration{ 980.0 };

	double Acceleration{ GravitationalAcceleration };
	Acceleration = GravitationalAcceleration * CompressionRatio;

	const double Force{ Mass * Acceleration };
	const FVector RayCastForce{ Hit.ImpactNormal * Force };

	Component->AddForceAtLocation(RayCastForce, RayCastStart);


	const double DragCompressionMin{ 0.25 };

	if (CompressionRatio < DragCompression) DragCompression = CompressionRatio;
	if (DragCompression < DragCompressionMin) DragCompression = DragCompressionMin;
}


void AMowerRC::ApplyDragToGroundedComponent(UPrimitiveComponent* Component)
{
	if (!Component) return;

	UE_LOG(LogTemp, Warning, TEXT("CompressionMin %f"), DragCompression);
	UE_LOG(LogTemp, Warning, TEXT("Linear %f	Angular %f"), Component->GetLinearDamping(), Component->GetAngularDamping());

	if (DragCompression == 1.0)
	{
		Component->SetLinearDamping(0.01);
		Component->SetAngularDamping(0.0);

		return;
	}
	
	const double MaxDrag{ 2.0 };

	double WheelDrag{ GroundedWheels / 4.0 };

	Component->SetLinearDamping(MaxDrag / DragCompression * WheelDrag);
	Component->SetAngularDamping(MaxDrag / DragCompression * WheelDrag);

	DragCompression = 1.0;
	GroundedWheels = 0;
}


void AMowerRC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		EnhancedInputComponent->BindAction(MoveCameraInputAction, ETriggerEvent::Triggered, this, &AMowerRC::MoveCamera);
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

