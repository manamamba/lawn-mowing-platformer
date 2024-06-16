// Derived AActor class AMovingPlatformA by Cody Wheeler.


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
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
}


void AMovingPlatformA::BeginPlay()
{
	Super::BeginPlay();

	if (!bActiveWithoutSpawner) SetActorTickEnabled(false);

	if (bMoves) 
		if (LocationOffset == FVector::Zero()) bMoves = false; 
		else SetMovingData();

	if (bRotates)
		if (RotationOffset == FRotator::ZeroRotator) bRotates = false;
		else SetRotatingData();
}

void AMovingPlatformA::SetMovingData()
{
	StartLocation = Root->GetComponentLocation();
	EndLocation = StartLocation + LocationOffset;
	MovingDistance = FVector::Dist(StartLocation, EndLocation);
	MovingDirection = FVector{ EndLocation - StartLocation }.GetSafeNormal();
}

void AMovingPlatformA::SetRotatingData()
{
	if (RotationOffset.Pitch == 0.0) bPitchOffsetIsZero = true;
	if (RotationOffset.Yaw == 0.0) bYawOffsetIsZero = true;
	if (RotationOffset.Roll == 0.0) bRollOffsetIsZero = true;
	
	if (!bPitchOffsetIsZero) RotatingDirection.Pitch = RotationOffset.Pitch / abs(RotationOffset.Pitch);
	if (!bYawOffsetIsZero) RotatingDirection.Yaw = RotationOffset.Yaw / abs(RotationOffset.Yaw);
	if (!bRollOffsetIsZero) RotatingDirection.Roll = RotationOffset.Roll / abs(RotationOffset.Roll);
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
	TimeWaitedBeforeRotating += DeltaTime;
	
	if (TimeWaitedBeforeRotating < WaitTimeInSecondsBeforeRotating) return;

	const double RotationRate{ RotationSpeed * DeltaTime };

	LocalRotation += RotatingDirection * RotationRate;

	if (!bContinuousRotation) UpdateNonContinuousLocalRotation();

	ResetFullAxisRotations(LocalRotation);

	Root->SetWorldRotation(LocalRotation);
}

void AMovingPlatformA::UpdateNonContinuousLocalRotation()
{
	if (!bPitchOffsetIsZero) if (abs(LocalRotation.Pitch) > abs(RotationOffset.Pitch)) LocalRotation.Pitch = RotationOffset.Pitch;
	if (!bYawOffsetIsZero) if (abs(LocalRotation.Yaw) > abs(RotationOffset.Yaw)) LocalRotation.Yaw = RotationOffset.Yaw;
	if (!bRollOffsetIsZero) if (abs(LocalRotation.Roll) > abs(RotationOffset.Roll)) LocalRotation.Roll = RotationOffset.Roll;

	if (LocalRotation == RotationOffset)
	{
		if (!bPitchOffsetIsZero) RotatingDirection.Pitch = -RotatingDirection.Pitch;
		if (!bYawOffsetIsZero) RotatingDirection.Yaw = -RotatingDirection.Yaw;
		if (!bRollOffsetIsZero) RotatingDirection.Roll = -RotatingDirection.Roll;

		RotationOffset.Pitch = -RotationOffset.Pitch;
		RotationOffset.Yaw = -RotationOffset.Yaw;
		RotationOffset.Roll = -RotationOffset.Roll;

		if (bRotatesOnce) SetActorTickEnabled(false);

		TimeWaitedBeforeRotating = 0.0f;
	}
}

void AMovingPlatformA::ResetFullAxisRotations(FRotator& Rotator) const
{
	const double RotationMaximum{ 360.0 };
	
	if (abs(Rotator.Pitch) >= RotationMaximum) Rotator.Pitch = 0.0;
	if (abs(Rotator.Yaw) >= RotationMaximum) Rotator.Yaw = 0.0;
	if (abs(Rotator.Roll) >= RotationMaximum) Rotator.Roll = 0.0;
}
