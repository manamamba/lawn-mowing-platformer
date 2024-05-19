// Fill out your copyright notice in the Description page of Project Settings.


#include "MowerRC.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"


AMowerRC::AMowerRC()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignComponentSubObjects();
	SetupComponentAttachments();
	SetComponentProperties();
}


void AMowerRC::CreateAndAssignComponentSubObjects()
{
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
	RootComponent = Body;
	Handle->SetupAttachment(RootComponent);
	CameraArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(CameraArm);
	FRWheel->SetupAttachment(RootComponent);
	FLWheel->SetupAttachment(RootComponent);
	BRWheel->SetupAttachment(RootComponent);
	BLWheel->SetupAttachment(RootComponent);
}


void AMowerRC::SetComponentProperties()
{
	Body->SetGenerateOverlapEvents(false);
	Body->SetSimulatePhysics(true);
	Body->SetUseCCD(true);

	Handle->SetRelativeLocation(FVector{ -22.0, 0.0, 0.0 });
	Handle->SetGenerateOverlapEvents(false);
	Handle->SetCollisionProfileName("NoCollision");

	SetWheelProperties(FRWheel, LocalFRWheelPosition);
	SetWheelProperties(FLWheel, LocalFLWheelPosition);
	SetWheelProperties(BRWheel, LocalBRWheelPosition);
	SetWheelProperties(BLWheel, LocalBLWheelPosition);

	CameraArm->SetRelativeLocation(FVector{ 0.0, 0.0, 10.0 });
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
	CameraArm->TargetArmLength = 200.0f;
}


void AMowerRC::SetWheelProperties(UStaticMeshComponent* Wheel, FVector Location)
{
	if (!Wheel) return;
	
	Wheel->SetRelativeLocation(Location);
	Wheel->SetGenerateOverlapEvents(false);
	Wheel->SetCollisionProfileName("NoCollision");
}


void AMowerRC::BeginPlay()
{
	Super::BeginPlay();

	// Body->SetCenterOfMass(FVector{ 0.0, 0.0, -15.0 });

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

	// const double MowerAcceleration{ GetMowerAcceleration() };

	TestMowerHovering();

	// ApplyForceToGroundedWheel(FRWheel, LocalFRWheelPosition, MowerAcceleration);
	// ApplyForceToGroundedWheel(FLWheel, LocalFLWheelPosition, MowerAcceleration);
	// ApplyForceToGroundedWheel(BRWheel, LocalBRWheelPosition, MowerAcceleration);
	// ApplyForceToGroundedWheel(BLWheel, LocalBLWheelPosition, MowerAcceleration);
}


void AMowerRC::TestMowerHovering()
{
	FHitResult Hit{};
	const double RayLength{ 8.9 };
	const FVector Start{ Body->GetComponentLocation() };
	const FVector DownVector{ -Body->GetUpVector() };
	const FVector End{ Start + (DownVector * RayLength) };

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	if (Grounded)
	{
		const double MowerAcceleration{ GetMowerAcceleration() };

		const float Mass{ Body->BodyInstance.GetBodyMass() }; // 18.570330
		const double Force{ Mass * MowerAcceleration };
		const FVector UpVector{ Body->GetUpVector() };

		// if acceleration is near 0, then we still need to apply the needed force to keep it hovering!
		// need a base line force always being applied when ground is detected

		Body->AddForce(UpVector * Force);

		UE_LOG(LogTemp, Warning, TEXT("Mass: %f"), Mass);
		UE_LOG(LogTemp, Warning, TEXT("Acceleration: %f"), MowerAcceleration);
		UE_LOG(LogTemp, Warning, TEXT("Force: %f"), Force);
	}
}


double AMowerRC::GetMowerAcceleration()
{
	const FVector FinalVelocity{ Body->GetComponentVelocity() };
	const FVector InitialVelocity{ LastTickVelocity };
	LastTickVelocity = FinalVelocity;

	return double{ FVector::Dist(FinalVelocity, InitialVelocity) / UGameplayStatics::GetWorldDeltaSeconds(this) };
}


void AMowerRC::ApplyForceToGroundedWheel(UStaticMeshComponent* Wheel, const FVector& WheelPosition, double Acceleration)
{
	FHitResult Hit{};

	if (IsWheelGrounded(Wheel, WheelPosition, Hit)) ApplyForceToWheel(Hit, Acceleration);
}


bool AMowerRC::IsWheelGrounded(UStaticMeshComponent* Wheel, const FVector& WheelPosition, FHitResult& Hit)
{
	if (!Wheel) return false;

	constexpr double RayLength{ 8.9 };
	double HitRay{};
	const FVector DownVector{ -Body->GetUpVector() };
	const FTransform BodyTransform{ Body->GetComponentTransform() };
	const FVector Start{ UKismetMathLibrary::TransformLocation(BodyTransform, WheelPosition) };
	const FVector End{ Start + (DownVector * RayLength) };

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	if (Grounded) HitRay = RayLength - FVector::Dist(Start, Hit.ImpactPoint);
	Wheel->SetWorldLocation(Start + (-DownVector * HitRay));

	// DrawDebugLine(GetWorld(), Start, End, FColor::Blue);
	// DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 6.0, 6, FColor::Blue);

	return Grounded;
}


void AMowerRC::ApplyForceToWheel(const FHitResult& Hit, double Acceleration)
{
	const float Mass{ Body->BodyInstance.GetBodyMass() };
	const double Force{ Mass * Acceleration };
	const FVector UpVector{ Body->GetUpVector() };

	Body->AddForceAtLocation(UpVector * Force, Hit.ImpactPoint);
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

