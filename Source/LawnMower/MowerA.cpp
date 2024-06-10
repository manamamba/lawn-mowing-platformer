// Derived APawn class AMowerA by Cody Wheeler.


#include "MowerA.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


AMowerA::AMowerA()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignComponentSubObjects();
	SetupComponentAttachments();
	SetNonWheelProperties();
	SetWheelProperties(FWheelSet{ WheelFr, AxisFr, SuspensionFr, LocationFr, TEXT("WheelFR") });
	SetWheelProperties(FWheelSet{ WheelFl, AxisFl, SuspensionFl, LocationFl, TEXT("WheelFL") });
	SetWheelProperties(FWheelSet{ WheelBr, AxisBr, SuspensionBr, LocationBr, TEXT("WheelBR") });
	SetWheelProperties(FWheelSet{ WheelBl, AxisBl, SuspensionBl, LocationBl, TEXT("WheelBL") });
}


void AMowerA::CreateAndAssignComponentSubObjects()
{
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
	WheelFr = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFR"));
	WheelFl = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFL"));
	WheelBr = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelBR"));
	WheelBl = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelBL"));
	AxisFr = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("AxisFR"));
	AxisFl = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("AxisFL"));
	AxisBr = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("AxisBR"));
	AxisBl = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("AxisBL"));
	SuspensionFr = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SuspensionFR"));
	SuspensionFl = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SuspensionFL"));
	SuspensionBr = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SuspensionBR"));
	SuspensionBl = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SuspensionBL"));
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
}


void AMowerA::SetupComponentAttachments()
{
	if (Body) RootComponent = Body;
	if (Handle) Handle->SetupAttachment(RootComponent);
	if (WheelFr) WheelFr->SetupAttachment(RootComponent);
	if (WheelFl) WheelFl->SetupAttachment(RootComponent);
	if (WheelBr) WheelBr->SetupAttachment(RootComponent);
	if (WheelBl) WheelBl->SetupAttachment(RootComponent);
	if (AxisFr) AxisFr->SetupAttachment(WheelFr);
	if (AxisFl) AxisFl->SetupAttachment(WheelFl);
	if (AxisBr) AxisBr->SetupAttachment(WheelBr);
	if (AxisBl) AxisBl->SetupAttachment(WheelBl);
	if (SuspensionFr) SuspensionFr->SetupAttachment(WheelFr);
	if (SuspensionFl) SuspensionFl->SetupAttachment(WheelFl);
	if (SuspensionBr) SuspensionBr->SetupAttachment(WheelBr);
	if (SuspensionBl) SuspensionBl->SetupAttachment(WheelBl);
	if (Arrow) Arrow->SetupAttachment(RootComponent);
	if (CameraArm) CameraArm->SetupAttachment(RootComponent);
	if (Camera) Camera->SetupAttachment(CameraArm);
}


void AMowerA::SetNonWheelProperties()
{
	if (!Body || !Handle || !CameraArm) return;

	SetMeshCollisionProperties(Body);
	SetMeshCollisionProperties(Handle);

	Handle->SetRelativeLocation(FVector{ -22.0, 0.0, 0.0 });

	CameraArm->SetRelativeLocation(FVector{ 0.0, 0.0, 10.0 });
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
	CameraArm->TargetArmLength = 200.0f;
}


void AMowerA::SetWheelProperties(FWheelSet Set)
{
	if (!Set.Wheel || !Set.Axis || !Set.Suspension) return;

	SetMeshCollisionProperties(Set.Wheel);

	Set.Wheel->SetRelativeLocation(Set.Location);

	Set.Axis->ComponentName1.ComponentName = Set.WheelName;
	Set.Axis->ComponentName2.ComponentName = TEXT("Body");
	Set.Axis->SetLinearXLimit(LCM_Free, 0.0f);
	Set.Axis->SetLinearYLimit(LCM_Free, 0.0f);
	Set.Axis->SetLinearZLimit(LCM_Free, 0.0f);
	Set.Axis->SetAngularSwing1Limit(ACM_Locked, 45.0f);
	Set.Axis->SetAngularTwistLimit(ACM_Locked, 45.0f);
	Set.Axis->SetAngularDriveMode(EAngularDriveMode::Type::TwistAndSwing);

	Set.Suspension->ComponentName1.ComponentName = Set.WheelName;
	Set.Suspension->ComponentName2.ComponentName = TEXT("Body");
	Set.Suspension->SetLinearZLimit(LCM_Limited, 3.0f);
	Set.Suspension->SetLinearPositionDrive(false, false, true);
	Set.Suspension->SetLinearDriveParams(10.0f, 1.0f, 0.0f);
}


void AMowerA::SetMeshCollisionProperties(UStaticMeshComponent* Mesh)
{
	if (!Mesh) return;

	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_Pawn);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Destructible, ECR_Block);

	if (Mesh == Handle) return;

	Mesh->SetSimulatePhysics(true);
	Mesh->SetUseCCD(true);
}


void AMowerA::BeginPlay()
{
	Super::BeginPlay();

	SetMeshMassOverrides();
	AddInputMappingContextToLocalPlayerSubsystem();
}


void AMowerA::SetMeshMassOverrides()
{
	Body->BodyInstance.SetMassOverride(30.0f);
	Handle->BodyInstance.SetMassOverride(0.001f);
	WheelFr->BodyInstance.SetMassOverride(5.0f);
	WheelFl->BodyInstance.SetMassOverride(5.0f);
	WheelBr->BodyInstance.SetMassOverride(5.0f);
	WheelBl->BodyInstance.SetMassOverride(5.0f);

	Body->SetCenterOfMass(FVector{ 0.0, 0.0, -15.0 });
}


void AMowerA::AddInputMappingContextToLocalPlayerSubsystem() const
{
	APlayerController* PlayerController{};
	UEnhancedInputLocalPlayerSubsystem* Subsystem{};

	PlayerController = Cast<APlayerController>(Controller);
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (PlayerController && Subsystem) Subsystem->AddMappingContext(MowerInputMappingContext, 0);
}


void AMowerA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ReduceWheelDrag();
}


void AMowerA::ReduceWheelDrag()
{
	if (WheelDrag == 0.0f) return;
	(WheelDrag > 0.0f) ? WheelDrag -= 0.25f : WheelDrag == 0.0f;
	SetRearWheelAngularDamping();

	// UE_LOG(LogTemp, Warning, TEXT("Wheel Drag: %f"), WheelDrag);
}


void AMowerA::SetRearWheelAngularDamping()
{
	WheelBr->SetAngularDamping(WheelDrag);
	WheelBl->SetAngularDamping(WheelDrag);
}


void AMowerA::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(MoveCameraInputAction, ETriggerEvent::Triggered, this, &AMowerA::MoveCamera);
		EnhancedInputComponent->BindAction(ResetCameraInputAction, ETriggerEvent::Triggered, this, &AMowerA::ResetCamera);

		EnhancedInputComponent->BindAction(AccelerateInputAction, ETriggerEvent::Triggered, this, &AMowerA::Accelerate);
		EnhancedInputComponent->BindAction(BrakeInputAction, ETriggerEvent::Triggered, this, &AMowerA::Brake);
		EnhancedInputComponent->BindAction(SteerInputAction, ETriggerEvent::Triggered, this, &AMowerA::Steer);
		EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &AMowerA::Jump);
	}
}


void AMowerA::MoveCamera(const FInputActionValue& Value)
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


void AMowerA::ResetCamera()
{
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
}


void AMowerA::Accelerate(const FInputActionValue& Value)
{
	const float InputDirection{ Value.Get<float>() };
	FVector ForwardVector{ Body->GetForwardVector() };
	Body->AddForce(ForwardVector * InputDirection * AccelerationPower, NAME_None, true);

	// UE_LOG(LogTemp, Warning, TEXT("%f %f %s"), Direction, Power, *ForwardVector.ToString());
}


void AMowerA::Brake(const FInputActionValue& Value)
{
	WheelDrag += Value.Get<float>();

	if (WheelDrag > MaxWheelDrag) WheelDrag = MaxWheelDrag;
	SetRearWheelAngularDamping();
}


void AMowerA::Steer(const FInputActionValue& Value)
{
	double SteeringDirection{ Value.Get<float>() };

	FRotator WheelFrRotation{ WheelFr->GetRelativeRotation() };
	FRotator WheelFlRotation{ WheelFl->GetRelativeRotation() };

	WheelFrRotation.Yaw += SteeringDirection * SteeringPower;
	WheelFlRotation.Yaw += SteeringDirection * SteeringPower;

	WheelFr->SetRelativeRotation(WheelFrRotation);
	WheelFl->SetRelativeRotation(WheelFlRotation);
}


void AMowerA::Jump(const FInputActionValue& Value)
{
	APawn* Mower{ GetController()->GetPawn() };

	if (!IsGrounded(Mower)) return;

	// UE_LOG(LogTemp, Warning, TEXT("Mower is Grounded!"));

	Body->AddImpulse(Body->GetUpVector() * 25000.0);
}


bool AMowerA::IsGrounded(APawn* Mower) const
{
	if (!Mower) return false;

	FHitResult Hit{};
	double TraceDistance{ 14.0 };

	FVector Start{ Mower->GetActorLocation() };
	FVector Direction{ -Mower->GetActorUpVector() * TraceDistance };
	FVector End{ Start + Direction };

	bool bGrounded{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	// DrawDebugSphere(GetWorld(), Start, 20.0f, 6, FColor::Orange, true);
	// DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true);
	// DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 15.0f, 6, FColor::Green, true);

	return bGrounded;
}