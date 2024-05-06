// Fill out your copyright notice in the Description page of Project Settings.


#include "MowerKart.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AMowerKart::AMowerKart()
{
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	FRWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FRWheel"));
	FLWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FLWheel"));
	BRWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BRWheel"));
	BLWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BLWheel"));
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	RootComponent = Body;

	FRWheel->SetupAttachment(RootComponent);
	FLWheel->SetupAttachment(RootComponent);
	BRWheel->SetupAttachment(RootComponent);
	BLWheel->SetupAttachment(RootComponent);
	BoxCollision->SetupAttachment(RootComponent);
	SpringArm->SetupAttachment(RootComponent);
	Arrow->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm);

	SpringArm->SetRelativeRotation(FRotator{ -40, 0, 0 });
	SpringArm->TargetArmLength = 500.0f;

	FRWheel->SetRelativeLocation(FVector{ 50, 60, -50 });
	FLWheel->SetRelativeLocation(FVector{ 50, -60, -50 });
	BRWheel->SetRelativeLocation(FVector{ -50, 60, -50 });
	BLWheel->SetRelativeLocation(FVector{ -50, -60, -50 });

	FRWheel->SetRelativeScale3D(FVector{ 0.1, 0.05, 0.1 });
	FLWheel->SetRelativeScale3D(FVector{ 0.1, 0.05, 0.1 });
	BRWheel->SetRelativeScale3D(FVector{ 0.1, 0.05, 0.1 });
	BLWheel->SetRelativeScale3D(FVector{ 0.1, 0.05, 0.1 });
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

