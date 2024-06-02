// Grass actor class developed by Cody Wheeler.


#include "Grass.h"
#include "Kismet/KismetMathLibrary.h"


AGrass::AGrass()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Root;

	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel3);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
}


void AGrass::BeginPlay()
{
	Super::BeginPlay();

	// RandomizeRotationAndScale();
	CreateAndAttachSpawningComponents();
	SetSpawningComponentPositions();
}


void AGrass::RandomizeRotationAndScale()
{
	const double SpawnPitch{ UKismetMathLibrary::RandomFloatInRange(0.0f, 5.0f) };
	const double SpawnYaw{ UKismetMathLibrary::RandomFloatInRange(0.0f, 359.0f) };
	const double SpawnRoll{ UKismetMathLibrary::RandomFloatInRange(0.0f, 5.0f) };

	const double SpawnScaleXy{ UKismetMathLibrary::RandomFloatInRange(1.0f, 1.2f) };
	const double SpawnScaleZ{ UKismetMathLibrary::RandomFloatInRange(2.5f, 3.5f) };

	if (!Mesh) return;

	Mesh->SetWorldRotation(FRotator{ SpawnPitch, SpawnYaw, SpawnRoll });
	// Mesh->SetWorldScale3D(FVector{ SpawnScaleXy, SpawnScaleXy, SpawnScaleZ });
}


void AGrass::CreateAndAttachSpawningComponents()
{
	Rotator = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));
	Spawner = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));

	if (!Rotator || !Spawner) return;

	Rotator->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Spawner->AttachToComponent(Rotator, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	AddInstanceComponent(Rotator);
	AddInstanceComponent(Spawner);
}


void AGrass::SetSpawningComponentPositions()
{
	RootTransform = Root->GetComponentTransform();

	RotatorRotation = FRotator{ 67.5, 0.0, 0.0 };

	Rotator->SetRelativeRotation(RotatorRotation);	
	Rotator->SetRelativeLocation(FVector{ 0.0, 0.0, 3.0 });
	Spawner->SetRelativeLocation(FVector{ 7.0f, 0.0, 0.0 });
}


void AGrass::LogComponentsAttachedAtRuntime()
{
	TArray<UActorComponent*> ActorComponent{ GetInstanceComponents() };

	for (UActorComponent* Component : ActorComponent)
	{
		if (Component) UE_LOG(LogTemp, Warning, TEXT("Component Name: %s"), *Component->GetName());
	}
}


void AGrass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickSlower();

	// TryToSpawnGrass();
	// UpdateRotatorRotation();

	DrawSpawningComponents();
}


void AGrass::TickSlower()
{
	++TickCount;

	if (TickCount >= 30.0f)
	{
		TryToSpawnGrass();
		UpdateRotatorRotation();

		TickCount = 0.0f;
	}
}


void AGrass::TryToSpawnGrass()
{
	FHitResult Hit{};
	
	if (GroundHitBySpawnerRayCast(Hit)) if (!GrassHitBySpawnerSweep(Hit)) SpawnGrass(Hit);
}


bool AGrass::GroundHitBySpawnerRayCast(FHitResult& Hit)
{
	const double RayCastLength{ 3.1 };

	const FVector RayCastStart{ Spawner->GetComponentLocation() };
	const FVector RayCastEnd{ RayCastStart + (-Spawner->GetUpVector() * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


bool AGrass::GrassHitBySpawnerSweep(FHitResult& Hit) const
{
	FHitResult SweepHit{};
	const FVector Impact{ Hit.ImpactPoint };
	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(2.0) };

	return GetWorld()->SweepSingleByChannel(SweepHit, Impact, Impact, FQuat::Identity, ECC_GameTraceChannel2, Sweeper);
}


void AGrass::SpawnGrass(FHitResult& Hit)
{
	const FRotator SpawnRotation{ Rotator->GetComponentRotation() };
	const FVector SpawnLocation{ Hit.ImpactPoint };

	AGrass* SpawnedGrass{ GetWorld()->SpawnActor<AGrass>(GrassClass, SpawnLocation, SpawnRotation) };
}


void AGrass::UpdateRotatorRotation()
{
	const double RotatorPitchRate{ -22.5 };
	const double RotatorYawRate{ 60.0 };

	const double RotatorPitchRange{ 67.5 };
	const double RotatorYawEnd{ 360.0 - RotatorYawRate };

	if (RotatorRotation.Pitch != -RotatorPitchRange)
	{
		RotatorRotation.Pitch += RotatorPitchRate;
	}
	else if (RotatorRotation.Yaw != RotatorYawEnd)
	{
		RotatorRotation.Yaw += RotatorYawRate;
		RotatorRotation.Pitch = RotatorPitchRange;
	}
	else
	{
		DestroySpawningComponentsAndDisableTick();
		return;
	}

	Rotator->SetWorldRotation(UKismetMathLibrary::TransformRotation(RootTransform, RotatorRotation));
}


void AGrass::DestroySpawningComponentsAndDisableTick()
{
	Spawner->DestroyComponent();
	Rotator->DestroyComponent();

	SetActorTickEnabled(false);
}


void AGrass::DrawSpawningComponents()
{
	const FVector SpawnerRayCastStart{ Spawner->GetComponentLocation() };
	const FVector SpawnerRayCastEnd{ Spawner->GetComponentLocation() + (-Spawner->GetUpVector() * 3.0) };

	DrawDebugSphere(GetWorld(), Rotator->GetComponentLocation(), 4.0f, 6, FColor::Orange);
	DrawDebugSphere(GetWorld(), Spawner->GetComponentLocation(), 1.0f, 6, FColor::Blue);
	DrawDebugLine(GetWorld(), SpawnerRayCastStart, SpawnerRayCastEnd, FColor::White);
	DrawDebugSphere(GetWorld(), SpawnerRayCastEnd, 1.0f, 6, FColor::White);
}
