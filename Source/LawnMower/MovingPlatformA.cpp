// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatformA.h"


AMovingPlatformA::AMovingPlatformA()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Root;

	Mesh->SetupAttachment(RootComponent);

	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
}


void AMovingPlatformA::BeginPlay()
{
	Super::BeginPlay();

	RootTransform = Root->GetComponentTransform();
	StartLocation = RootTransform.GetLocation();
	EndLocation = StartLocation + LocationOffset;
	MaxDistance = FVector::Dist(StartLocation, EndLocation);
	MovingDirection = FVector{ EndLocation - StartLocation }.GetSafeNormal();

	SetActorTickEnabled(false);
}


void AMovingPlatformA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	WaitTimeInSeconds += DeltaTime;

	if (WaitTimeInSeconds < MaxWaitTimeInSeconds) return;

	if (bMovingToEndLocation) UpdateLocation(StartLocation, EndLocation, DeltaTime);
	else UpdateLocation(EndLocation, StartLocation, DeltaTime);
}

void AMovingPlatformA::UpdateLocation(const FVector& Origin, const FVector& Target, const float DeltaTime)
{
	FVector PlatformLocation{ Root->GetComponentLocation() };

	PlatformLocation += MovingDirection * (MovingSpeed * DeltaTime);
	
	if (FVector::Dist(PlatformLocation, Origin) >= MaxDistance)
	{
		if (bMoveOnce) SetActorTickEnabled(false);

		PlatformLocation = Target;
		MovingDirection = -MovingDirection;
		bMovingToEndLocation = !bMovingToEndLocation;
		WaitTimeInSeconds = 0.0f;
	}

	Root->SetWorldLocation(PlatformLocation);
}
