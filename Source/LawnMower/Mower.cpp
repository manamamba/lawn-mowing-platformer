// Fill out your copyright notice in the Description page of Project Settings.


#include "Mower.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"


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

	CameraArm->SetRelativeLocation(FVector{ -65.0, 0.0, 24.0 });
	CameraArm->SetRelativeRotation(FRotator{ -20.0, 0.0, 0.0 });
	CameraArm->TargetArmLength = 200.0f;

	SetWheelProperties(WheelSet{ WheelFR, AxisFR, SuspensionFR, FRPlacement, TEXT("WheelFR") });
	SetWheelProperties(WheelSet{ WheelFL, AxisFL, SuspensionFL, FLPlacement, TEXT("WheelFL") });
	SetWheelProperties(WheelSet{ WheelBR, AxisBR, SuspensionBR, BRPlacement, TEXT("WheelBR") });
	SetWheelProperties(WheelSet{ WheelBL, AxisBL, SuspensionBL, BLPlacement, TEXT("WheelBL") });
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

	WheelSet.Axis->ComponentName1.ComponentName = WheelSet.Name;
	WheelSet.Axis->ComponentName2.ComponentName = WheelSet.RootName;
	WheelSet.Axis->SetLinearXLimit(LCM_Free, 0.0f);
	WheelSet.Axis->SetLinearYLimit(LCM_Free, 0.0f);
	WheelSet.Axis->SetLinearZLimit(LCM_Free, 0.0f);
	WheelSet.Axis->SetAngularSwing1Limit(ACM_Locked, 45.0f);
	WheelSet.Axis->SetAngularTwistLimit(ACM_Locked, 45.0f);
	WheelSet.Axis->SetAngularDriveMode(EAngularDriveMode::Type::TwistAndSwing);

	WheelSet.Suspension->ComponentName1.ComponentName = WheelSet.Name;
	WheelSet.Suspension->ComponentName2.ComponentName = WheelSet.RootName;
	WheelSet.Suspension->SetLinearZLimit(LCM_Limited, 3.0f);
	WheelSet.Suspension->SetAngularTwistLimit(ACM_Locked, 45.0f);
}


void AMower::BeginPlay()
{
	Super::BeginPlay();
}


void AMower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AMower::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

