// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatformA.h"

#include "Kismet/KismetMathLibrary.h"


AMovingPlatformA::AMovingPlatformA()
{
	PrimaryActorTick.bCanEverTick = true;

	SetComponentProperties();
}

void AMovingPlatformA::SetComponentProperties()
{
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

	if (!bActiveWithoutSpawner) SetActorTickEnabled(false);

	if (bMoves) SetMovingData();

	if (bRotates) SetRotationData();
}

void AMovingPlatformA::SetMovingData()
{
	StartLocation = RootTransform.GetLocation();
	EndLocation = StartLocation + LocationOffset;
	MovingDistance = FVector::Dist(StartLocation, EndLocation);
	MovingDirection = FVector{ EndLocation - StartLocation }.GetSafeNormal();
}

void AMovingPlatformA::SetRotationData()
{
	RotatingDirections.Pitch = RotationOffset.Pitch / abs(RotationOffset.Pitch);
	RotatingDirections.Yaw = RotationOffset.Yaw / abs(RotationOffset.Yaw);
	RotatingDirections.Roll = RotationOffset.Roll / abs(RotationOffset.Roll);
}

void AMovingPlatformA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMoves) TryToMovePlatform(DeltaTime);

	if (bRotates) TryToRotatePlatform(DeltaTime);
}

void AMovingPlatformA::TryToMovePlatform(const float DeltaTime)
{
	TimeWaitedBeforeMoving += DeltaTime;
	
	if (TimeWaitedBeforeMoving < WaitTimeInSecondsBeforeMoving) return;

	if (bMovingToEndLocation) UpdateLocation(StartLocation, EndLocation, DeltaTime);
	else UpdateLocation(EndLocation, StartLocation, DeltaTime);
}

void AMovingPlatformA::UpdateLocation(const FVector& Origin, const FVector& Target, const float DeltaTime)
{
	FVector PlatformLocation{ Root->GetComponentLocation() };

	PlatformLocation += MovingDirection * (MovingSpeed * DeltaTime);
	
	if (FVector::Dist(PlatformLocation, Origin) >= MovingDistance)
	{
		if (bMovesOnce) SetActorTickEnabled(false);

		PlatformLocation = Target;
		MovingDirection = -MovingDirection;
		bMovingToEndLocation = !bMovingToEndLocation;
		TimeWaitedBeforeMoving = 0.0f;
	}

	Root->SetWorldLocation(PlatformLocation);
}

void AMovingPlatformA::TryToRotatePlatform(const float DeltaTime)
{
	const double RotationRate{ RotationSpeed * DeltaTime };

	if (RotationOffset.Pitch != 0.0) LocalRotation.Pitch += RotatingDirections.Pitch * RotationRate;
	if (RotationOffset.Yaw != 0.0) LocalRotation.Pitch += RotatingDirections.Yaw * RotationRate;
	if (RotationOffset.Roll != 0.0) LocalRotation.Pitch += RotatingDirections.Roll * RotationRate;

	// if (LocalRotation.Pitch )



	// if !bContinuousRotation 
	// RotationOffset = -RotationOffset;

	Root->SetWorldRotation(UKismetMathLibrary::TransformRotation(Root->GetComponentTransform(), LocalRotation));
}

void AMovingPlatformA::ResetFullAxisRotations(FRotator& Rotator) const
{
	const double RotationMaximum{ 360.0 };
	
	if (abs(Rotator.Pitch) >= RotationMaximum) Rotator.Pitch = 0.0;
	if (abs(Rotator.Yaw) >= RotationMaximum) Rotator.Yaw = 0.0;
	if (abs(Rotator.Roll) >= RotationMaximum) Rotator.Roll = 0.0;
}