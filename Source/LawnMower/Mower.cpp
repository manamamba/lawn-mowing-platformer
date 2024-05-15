// Fill out your copyright notice in the Description page of Project Settings.


#include "Mower.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


AMower::AMower()
{
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
	WheelFR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFR"));
	WheelFL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFL"));
	WheelBR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelBR"));
	WheelBL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelBL"));
	AxisFR = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("AxisFR"));
	AxisFL = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("AxisFL"));
	AxisBR = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("AxisBR"));
	AxisBL = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("AxisBL"));
	SuspensionFR = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SuspensionFR"));
	SuspensionFL = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SuspensionFL"));
	SuspensionBR = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SuspensionBR"));
	SuspensionBL = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SuspensionBL"));
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	RootComponent = Body;
	Handle->SetupAttachment(RootComponent);
	WheelFR->SetupAttachment(RootComponent);
	WheelFL->SetupAttachment(RootComponent);
	WheelBR->SetupAttachment(RootComponent);
	WheelBL->SetupAttachment(RootComponent);
	AxisFR->SetupAttachment(WheelFR);
	AxisFL->SetupAttachment(WheelFL);
	AxisBR->SetupAttachment(WheelBR);
	AxisBL->SetupAttachment(WheelBL);
	SuspensionFR->SetupAttachment(WheelFR);
	SuspensionFL->SetupAttachment(WheelFL);
	SuspensionBR->SetupAttachment(WheelBR);
	SuspensionBL->SetupAttachment(WheelBL);
	Arrow->SetupAttachment(RootComponent);
	CameraArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(CameraArm);

	Body->SetSimulatePhysics(true);
	Body->SetMassOverrideInKg(NAME_None, 30.0f);
	Body->SetCenterOfMass(FVector{ 0.0, 0.0, -30.0 });
	Body->SetUseCCD(true);
	Body->SetCollisionProfileName(TEXT("Pawn"));

	Handle->SetRelativeLocation(FVector{ -22.0, 0.0, 0.0 });
	Handle->SetCollisionProfileName(TEXT("Custom..."));
	Handle->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	Handle->SetCollisionObjectType(ECC_WorldDynamic);
	Handle->SetCollisionResponseToAllChannels(ECR_Block);
	Handle->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	SetWheelProperties(WheelSet{ WheelFR, AxisFR, SuspensionFR, FRPlacement, TEXT("WheelFR") });
	SetWheelProperties(WheelSet{ WheelFL, AxisFL, SuspensionFL, FLPlacement, TEXT("WheelFL") });
	SetWheelProperties(WheelSet{ WheelBR, AxisBR, SuspensionBR, BRPlacement, TEXT("WheelBR") });
	SetWheelProperties(WheelSet{ WheelBL, AxisBL, SuspensionBL, BLPlacement, TEXT("WheelBL") });

	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
	CameraArm->TargetArmLength = 200.0f;
}


void AMower::SetWheelProperties(WheelSet WheelSet)
{
	WheelSet.Wheel->SetRelativeLocation(WheelSet.Location);
	WheelSet.Wheel->SetSimulatePhysics(true);
	WheelSet.Wheel->SetMassOverrideInKg(NAME_None, 5.0f);
	WheelSet.Wheel->SetUseCCD(true);
	WheelSet.Wheel->SetCollisionProfileName(TEXT("Custom..."));
	WheelSet.Wheel->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	WheelSet.Wheel->SetCollisionObjectType(ECC_PhysicsBody);
	WheelSet.Wheel->SetCollisionResponseToAllChannels(ECR_Block);
	WheelSet.Wheel->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	WheelSet.Axis->ComponentName1.ComponentName = WheelSet.WheelName;
	WheelSet.Axis->ComponentName2.ComponentName = WheelSet.RootName;
	WheelSet.Axis->SetLinearXLimit(LCM_Free, 0.0f);
	WheelSet.Axis->SetLinearYLimit(LCM_Free, 0.0f);
	WheelSet.Axis->SetLinearZLimit(LCM_Free, 0.0f);
	WheelSet.Axis->SetAngularSwing1Limit(ACM_Locked, 45.0f);
	WheelSet.Axis->SetAngularTwistLimit(ACM_Locked, 45.0f);
	WheelSet.Axis->SetAngularDriveMode(EAngularDriveMode::Type::TwistAndSwing);

	WheelSet.Suspension->ComponentName1.ComponentName = WheelSet.WheelName;
	WheelSet.Suspension->ComponentName2.ComponentName = WheelSet.RootName;
	WheelSet.Suspension->SetLinearZLimit(LCM_Limited, 3.0f);
	WheelSet.Suspension->SetAngularTwistLimit(ACM_Locked, 45.0f);
}


void AMower::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}


void AMower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ResetFrontWheelRotations(DeltaTime);
}


void AMower::ResetFrontWheelRotations(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Rotation: %f"), WheelFR->GetRelativeRotation().Yaw);
	
	if (SteeringActive) return;

	FRotator Rotation{ WheelFR->GetRelativeRotation() };
	
	if (Rotation.Yaw == 0.0) return;

	if (Rotation.Yaw < -1.0) Rotation.Yaw += 1.0 * DeltaTime;
	else if (Rotation.Yaw > 1.0) Rotation.Yaw -= 1.0 * DeltaTime;
	else Rotation.Yaw = 0.0;

	WheelFR->SetRelativeRotation(Rotation);
	WheelFL->SetRelativeRotation(Rotation);
}


void AMower::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(MoveCameraInputAction, ETriggerEvent::Triggered, this, &AMower::MoveCamera);
		EnhancedInputComponent->BindAction(ResetCameraInputAction, ETriggerEvent::Triggered, this, &AMower::ResetCamera);

		EnhancedInputComponent->BindAction(AccelerateInputAction, ETriggerEvent::Triggered, this, &AMower::Accelerate);
		EnhancedInputComponent->BindAction(BrakeOnInputAction, ETriggerEvent::Triggered, this, &AMower::BrakeOn);
		EnhancedInputComponent->BindAction(BrakeOffInputAction, ETriggerEvent::Triggered, this, &AMower::BrakeOff);
		EnhancedInputComponent->BindAction(SteerInputAction, ETriggerEvent::Triggered, this, &AMower::Steer);
	}
}


void AMower::MoveCamera(const FInputActionValue& Value)
{
	FVector2D LookAxisVector{ Value.Get<FVector2D>() };
	FRotator Rotation{ CameraArm->GetRelativeRotation() };

	Rotation.Yaw += LookAxisVector.X;

	if (Rotation.Pitch + LookAxisVector.Y > 1.5) Rotation.Pitch = 1.5;
	else if (Rotation.Pitch + LookAxisVector.Y < -89) Rotation.Pitch = -89;
	else Rotation.Pitch += LookAxisVector.Y;

	Rotation.Roll = 0;

	// UE_LOG(LogTemp, Warning, TEXT("Camera Rotation: %s"), *Rotation.ToString());

	CameraArm->SetRelativeRotation(Rotation);
}


void AMower::ResetCamera()
{
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
}


void AMower::Accelerate(const FInputActionValue& Value)
{
	float ForceDirection{ Value.Get<float>() };
	FVector ForwardVector{ Body->GetForwardVector() };
	double ForcePower{ 300.0 };

	// UE_LOG(LogTemp, Warning, TEXT("%f %f %s"), Direction, Power, *ForwardVector.ToString());

	Body->AddForce(ForwardVector * ForcePower * ForceDirection, NAME_None, true);
	
}


void AMower::BrakeOn()
{
	// somehow combine as one function with press and release in imc to set damping?
	
	float DragForce{ 20.0f };

	WheelFR->SetAngularDamping(DragForce);
	WheelFL->SetAngularDamping(DragForce);
	WheelBR->SetAngularDamping(DragForce);
	WheelBL->SetAngularDamping(DragForce);
}


void AMower::BrakeOff()
{
	WheelFR->SetAngularDamping(0.0f);
	WheelFL->SetAngularDamping(0.0f);
	WheelBR->SetAngularDamping(0.0f);
	WheelBL->SetAngularDamping(0.0f);
}


void AMower::Steer(const FInputActionValue& Value)
{
	SteeringActive = true;

	float SteeringDirection{ Value.Get<float>() };
	FRotator Rotation{ WheelFR->GetComponentRotation() };

	double SteeringPower{ 10.0 };

	Rotation.Yaw += SteeringDirection * SteeringPower;

	UE_LOG(LogTemp, Warning, TEXT("Yaw: %f"), Rotation.Yaw);
	
	WheelFR->SetRelativeRotation(Rotation);
	WheelFL->SetRelativeRotation(Rotation);

	SteeringActive = false;

	// reset damping on wheels
}

