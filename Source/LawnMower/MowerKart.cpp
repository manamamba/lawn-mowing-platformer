// Fill out your copyright notice in the Description page of Project Settings.


#include "MowerKart.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"


AMowerKart::AMowerKart()
{
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	FrontRightWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontRightWheel"));
	FrontLeftWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontLeftWheel"));
	BackRightWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackRightWheel"));
	BackLeftWheel= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackLeftWheel"));

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	RootComponent = Body;
	BoxCollision->SetupAttachment(RootComponent);
	FrontRightWheel->SetupAttachment(RootComponent);
	FrontLeftWheel->SetupAttachment(RootComponent);
	BackRightWheel->SetupAttachment(RootComponent);
	BackLeftWheel->SetupAttachment(RootComponent);
	SpringArm->SetupAttachment(RootComponent);
	Arrow->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm);

	Body->SetSimulatePhysics(true);
	FrontRightWheel->SetSimulatePhysics(true);
	FrontLeftWheel->SetSimulatePhysics(true);
	BackRightWheel->SetSimulatePhysics(true);
	BackLeftWheel->SetSimulatePhysics(true);

	SpringArm->SetRelativeRotation(FRotator{ -40, 0, 0 });
	SpringArm->TargetArmLength = 500.0f;
}


void AMowerKart::BeginPlay()
{
	Super::BeginPlay();
}


void AMowerKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AMowerKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

