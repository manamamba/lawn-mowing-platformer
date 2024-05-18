// Fill out your copyright notice in the Description page of Project Settings.


#include "MowerRC.h"
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

	SetWheelProperties(FRWheel, FVector{ 24.0, 24.0, -8.0 });
	SetWheelProperties(FLWheel, FVector{ 24.0, -24.0, -8.0 });
	SetWheelProperties(BRWheel, FVector{ -24.0, 24.0, -8.0 });
	SetWheelProperties(BLWheel, FVector{ -24.0, -24.0, -8.0 });

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
	
	OverrideMass();
	AddInputMappingContextToLocalPlayerSubsystem();
}


void AMowerRC::OverrideMass()
{
	Body->BodyInstance.SetMassOverride(MowerMass);
	Body->SetCenterOfMass(FVector{ 0.0, 0.0, -15.0 });
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


	double MowerVelocity{ Body->GetComponentVelocity().Z };







	ApplyForceToGroundedWheel(FRWheel);
	ApplyForceToGroundedWheel(FLWheel);
	ApplyForceToGroundedWheel(BRWheel);
	ApplyForceToGroundedWheel(BLWheel);
}


void AMowerRC::ApplyForceToGroundedWheel(UStaticMeshComponent* Wheel)
{
	RayCastResponse WheelResponse{};

	if (!IsWheelGrounded(Wheel, WheelResponse)) return;

	// UE_LOG(LogTemp, Warning, TEXT("%s is Grounded!"), *Wheel->GetName() );

	ApplyForceToWheel(WheelResponse);

	// UE_LOG(LogTemp, Warning, TEXT("%s Force Applied!"), *Wheel->GetName() );
}


bool AMowerRC::IsWheelGrounded(UStaticMeshComponent* Wheel, RayCastResponse& WheelResponse)
{
	if (!Wheel) return false;
	
	constexpr double RayLength{ 9.0 };
	const FVector DownVector{ -Body->GetUpVector() };
	const FVector Start{ Wheel->GetComponentLocation() };
	const FVector End{ Start + (DownVector * RayLength) };

	bool Grounded{ GetWorld()->LineTraceSingleByChannel(WheelResponse.Hit, Start, End, ECC_GameTraceChannel1) };

	if (Grounded) WheelResponse.HitRayDifference = RayLength - FVector::Dist(WheelResponse.Hit.ImpactPoint, Start);
	if (Grounded) UE_LOG(LogTemp, Warning, TEXT("ImpactDistance: %f"), WheelResponse.HitRayDifference);
	// DrawDebugLine(GetWorld(), Start, End, FColor::Purple);
	DrawDebugSphere(GetWorld(), WheelResponse.Hit.ImpactPoint, 6.0, 6, FColor::Purple);

	return Grounded;
}


void AMowerRC::ApplyForceToWheel(const RayCastResponse& WheelResponse)
{









	//constexpr double Force{ 20030.0 };
	//const FVector UpVector{ Body->GetUpVector() };
	//double Force{ 30.0 * -MowerDownVelocity };
	//Body->AddForceAtLocation(UpVector * Force, WheelResponse.Hit.ImpactPoint);
}


void AMowerRC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

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

