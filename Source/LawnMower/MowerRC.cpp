// Fill out your copyright notice in the Description page of Project Settings.


#include "MowerRC.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

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

	// ApplyForceToGroundedMower(GetMowerAcceleration(DeltaTime));

	Float(DeltaTime);

	//const double MowerAcceleration{ GetMowerAcceleration() };
	// ApplyForceToWheelPosition(FRWheel, LocalFRWheelPosition);
	// ApplyForceToWheelPosition(FLWheel, LocalFLWheelPosition);
	// ApplyForceToWheelPosition(BRWheel, LocalBRWheelPosition);
	// ApplyForceToWheelPosition(BLWheel, LocalBLWheelPosition);
}


void AMowerRC::Float(float DeltaTime)
{
	const double Mass{ Body->GetMass() };
	const double Force{ Mass * GravitationalAcceleration };

	Body->AddForce(FVector::UpVector * Force);

	double Acceleration{ GetMowerAcceleration(DeltaTime) };
	FVector VelocityDirection{ FinalVelocity.GetSafeNormal(0.01) };
	const FVector Start{ Body->GetComponentLocation() };
	const FVector End{ Start + (-VelocityDirection * 100.0) };
	double AntiVelocityForce{ Mass * Acceleration };

	// if acceleration is over a threshold, do not apply opposite velocity
	if (Acceleration < GravitationalAcceleration) Body->AddForce(-VelocityDirection * AntiVelocityForce);

	// need to get its rotation back to upright and stopped

	
	DrawDebugLine(GetWorld(), Start, End, FColor::Blue);

	UE_LOG(LogTemp, Warning, TEXT("Acceleration      %f"), Acceleration);
	UE_LOG(LogTemp, Warning, TEXT("AntiVelocityForce %f"), AntiVelocityForce);
	UE_LOG(LogTemp, Warning, TEXT("VelocityDirection %s"), *VelocityDirection.ToString());
}


double AMowerRC::GetMowerAcceleration(float DeltaTime)
{
	FinalVelocity = Body->GetComponentVelocity();
	InitialVelocity = LastTickVelocity;
	LastTickVelocity = FinalVelocity;
	
	return double{ FVector::Dist(FinalVelocity, InitialVelocity) / DeltaTime };
}


void AMowerRC::ApplyForceToGroundedMower(double Acceleration)
{
	FHitResult Hit{};

	constexpr double RayLength{ 15.9 };
	double HitRay{};

	const FVector UpVector{ Body->GetUpVector() };
	const FVector Start{ Body->GetComponentLocation() };
	const FVector End{ Start + (-UpVector * RayLength) };

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	if (!Grounded) return;

	DrawDebugLine(GetWorld(), Start, End, FColor::Orange);
	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 2.0, 6, FColor::Orange);

	FVector VelocityDirection{ FinalVelocity.GetSafeNormal(1.0) };
	const double Force{ Body->GetMass() * Acceleration };
	const double AntiGravitationalForce{ Body->GetMass() * GravitationalAcceleration };

	Body->AddForce(UpVector * (AntiGravitationalForce / Force) );



	DrawDebugLine(GetWorld(), Start, Start + VelocityDirection * 50.0, FColor::Purple);
	DrawDebugSphere(GetWorld(), Start + VelocityDirection * 50.0, 15.0, 12, FColor::Purple);
	UE_LOG(LogTemp, Warning, TEXT("Acceleration %f"), Acceleration);
	UE_LOG(LogTemp, Warning, TEXT("Force %f"), Force);
}






















void AMowerRC::ApplyForceToWheelPosition(UStaticMeshComponent* Wheel, const FVector& WheelPosition)
{
	if (!Wheel) return;
	
	FHitResult Hit{};

	constexpr double RayLength{ 8.9 };
	double HitRay{};

	const FVector UpVector{ Body->GetUpVector() };
	const FTransform BodyTransform{ Body->GetComponentTransform() };
	const FVector Start{ UKismetMathLibrary::TransformLocation(BodyTransform, WheelPosition) };
	const FVector End{ Start + (-UpVector * RayLength) };

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	if (Grounded) HitRay = RayLength - FVector::Dist(Start, Hit.ImpactPoint);

	Wheel->SetWorldLocation(Start + (UpVector * HitRay));

	// DrawDebugSphere(GetWorld(), Start, 2.0, 6, FColor::Blue);
	// DrawDebugLine(GetWorld(), Start, End, FColor::Purple);
	// DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 3.0, 6, FColor::Orange);

	if (!Grounded) return;

	FVector NewWheelPosition{ Wheel->GetComponentLocation() };

	// Float();

	const double WheelCount{ 4.0 };
	const float Mass{ Body->GetMass() };

	double Force{ (Mass * GravitationalAcceleration) / WheelCount };
	
	// DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + (UpVector * 20.0), FColor::Green);
	// UE_LOG(LogTemp, Warning, TEXT("%s Force Applied: %f"), *Wheel->GetName(), Force);

	Body->AddForceAtLocation(FVector::UpVector * Force, Hit.ImpactPoint);
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

