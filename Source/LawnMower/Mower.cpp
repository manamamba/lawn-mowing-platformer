// Fill out your copyright notice in the Description page of Project Settings.


#include "Mower.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Kismet/KismetMathLibrary.h"


AMower::AMower()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignComponentSubObjects();
	SetupComponentAttachments();
	SetNonWheelProperties();
	SetWheelProperties(WheelSet{ WheelFR, AxisFR, SuspensionFR, LocationFR, TEXT("WheelFR") });
	SetWheelProperties(WheelSet{ WheelFL, AxisFL, SuspensionFL, LocationFL, TEXT("WheelFL") });
	SetWheelProperties(WheelSet{ WheelBR, AxisBR, SuspensionBR, LocationBR, TEXT("WheelBR") });
	SetWheelProperties(WheelSet{ WheelBL, AxisBL, SuspensionBL, LocationBL, TEXT("WheelBL") });
}


void AMower::CreateAndAssignComponentSubObjects()
{
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
}


void AMower::SetupComponentAttachments()
{
	if (Body) RootComponent = Body;
	if (Handle) Handle->SetupAttachment(RootComponent);
	if (WheelFR) WheelFR->SetupAttachment(RootComponent);
	if (WheelFL) WheelFL->SetupAttachment(RootComponent);
	if (WheelBR) WheelBR->SetupAttachment(RootComponent);
	if (WheelBL) WheelBL->SetupAttachment(RootComponent);
	if (AxisFR) AxisFR->SetupAttachment(WheelFR);
	if (AxisFL) AxisFL->SetupAttachment(WheelFL);
	if (AxisBR) AxisBR->SetupAttachment(WheelBR);
	if (AxisBL) AxisBL->SetupAttachment(WheelBL);
	if (SuspensionFR) SuspensionFR->SetupAttachment(WheelFR);
	if (SuspensionFL) SuspensionFL->SetupAttachment(WheelFL);
	if (SuspensionBR) SuspensionBR->SetupAttachment(WheelBR);
	if (SuspensionBL) SuspensionBL->SetupAttachment(WheelBL);
	if (Arrow) Arrow->SetupAttachment(RootComponent);
	if (CameraArm) CameraArm->SetupAttachment(RootComponent);
	if (Camera) Camera->SetupAttachment(CameraArm);
}


void AMower::SetNonWheelProperties()
{
	if (Body == nullptr || Handle == nullptr || CameraArm == nullptr) return;

	Body->SetSimulatePhysics(true);
	Body->SetMassOverrideInKg(NAME_None, 30.0f);
	Body->SetCenterOfMass(FVector{ 0.0, 0.0, -15.0 });
	Body->SetUseCCD(true);
	Body->SetCollisionProfileName(TEXT("Pawn"));

	Handle->SetRelativeLocation(FVector{ -22.0, 0.0, 0.0 });
	Handle->SetCollisionProfileName(TEXT("Custom..."));
	Handle->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	Handle->SetCollisionObjectType(ECC_WorldDynamic);
	Handle->SetCollisionResponseToAllChannels(ECR_Block);
	Handle->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	CameraArm->SetRelativeLocation(FVector{ 0.0, 0.0, 10.0 });
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
	CameraArm->TargetArmLength = 200.0f;
}


void AMower::SetWheelProperties(WheelSet Set)
{
	if (Set.Wheel == nullptr || Set.Axis == nullptr || Set.Suspension == nullptr) return;
	
	Set.Wheel->SetRelativeLocation(Set.Location); // reset frwheel location in bp for test
	Set.Wheel->SetSimulatePhysics(true);
	Set.Wheel->SetMassOverrideInKg(NAME_None, 5.0f);
	Set.Wheel->SetUseCCD(true);
	Set.Wheel->SetCollisionProfileName(TEXT("Custom..."));
	Set.Wheel->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	Set.Wheel->SetCollisionObjectType(ECC_PhysicsBody);
	Set.Wheel->SetCollisionResponseToAllChannels(ECR_Block);
	Set.Wheel->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	Set.Axis->ComponentName1.ComponentName = Set.WheelName;
	Set.Axis->ComponentName2.ComponentName = Set.RootName;
	Set.Axis->SetLinearXLimit(LCM_Free, 0.0f);
	Set.Axis->SetLinearYLimit(LCM_Free, 0.0f);
	Set.Axis->SetLinearZLimit(LCM_Free, 0.0f);
	Set.Axis->SetAngularSwing1Limit(ACM_Locked, 45.0f);
	Set.Axis->SetAngularTwistLimit(ACM_Locked, 45.0f);
	Set.Axis->SetAngularDriveMode(EAngularDriveMode::Type::TwistAndSwing);

	Set.Suspension->ComponentName1.ComponentName = Set.WheelName;
	Set.Suspension->ComponentName2.ComponentName = Set.RootName;
	Set.Suspension->SetLinearZLimit(LCM_Limited, 3.0f);
}


void AMower::BeginPlay()
{
	Super::BeginPlay();

	AddMappingContextToLocalPlayerSubsystem();
}


void AMower::AddMappingContextToLocalPlayerSubsystem()
{
	APlayerController* PlayerController{};
	UEnhancedInputLocalPlayerSubsystem* Subsystem{};

	PlayerController = Cast<APlayerController>(Controller);
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (PlayerController && Subsystem) Subsystem->AddMappingContext(InputMappingContext, 0);
}


void AMower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	const FVector2D InputVector{ Value.Get<FVector2D>() };
	FRotator ArmPosition{ CameraArm->GetRelativeRotation() };

	ArmPosition.Yaw += InputVector.X;
	ArmPosition.Pitch += InputVector.Y;
	if (ArmPosition.Pitch > MaxArmPitch) ArmPosition.Pitch = MaxArmPitch;
	if (ArmPosition.Pitch < MinArmPitch) ArmPosition.Pitch = MinArmPitch;

	CameraArm->SetRelativeRotation(ArmPosition);

	// UE_LOG(LogTemp, Warning, TEXT("Camera Rotation: %s"), *ArmPosition.ToString());
}


void AMower::ResetCamera()
{
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
}


void AMower::Accelerate(const FInputActionValue& Value)
{
	const float InputDirection{ Value.Get<float>()};
	FVector ForwardVector{ Body->GetForwardVector() };

	Body->AddForce(ForwardVector * InputDirection * AccelerationPower, NAME_None, true);

	// UE_LOG(LogTemp, Warning, TEXT("%f %f %s"), Direction, Power, *ForwardVector.ToString());
}


void AMower::BrakeOn(const FInputActionValue& Value)
{
	WheelDrag += Value.Get<float>() * 0.5f;
	if (WheelDrag > MaxWheelDrag) WheelDrag = MaxWheelDrag;

	SetWheelDrag();
}


void AMower::BrakeOff()
{
	WheelDrag = 0.0f;

	SetWheelDrag();
}


void AMower::SetWheelDrag()
{
	WheelBR->SetAngularDamping(WheelDrag);
	WheelBL->SetAngularDamping(WheelDrag);
}


void AMower::Steer(const FInputActionValue& Value)
{
	float SteeringDirection{ Value.Get<float>() };

	FRotator WheelFRRotation{ WheelFR->GetRelativeRotation() };
	FRotator WheelFLRotation{ WheelFL->GetRelativeRotation() };

	WheelFRRotation.Yaw += SteeringDirection * SteeringPower;
	WheelFLRotation.Yaw += SteeringDirection * SteeringPower;

	WheelFR->SetRelativeRotation(WheelFRRotation);
	WheelFL->SetRelativeRotation(WheelFLRotation);
}

