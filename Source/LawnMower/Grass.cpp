// Grass actor class developed by Cody Wheeler.


#include "Grass.h"
#include "Components/BoxComponent.h"
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

	RandomizeRotationAndScale();
	CreateAndAttachSpawningComponents();
	SetSpawningComponentPositions();

	// LogComponentsAttachedAtRuntime();
}


void AGrass::RandomizeRotationAndScale()
{
	const double SpawnPitch{ UKismetMathLibrary::RandomFloatInRange(0.0f, 10.0f) };
	const double SpawnYaw{ UKismetMathLibrary::RandomFloatInRange(0.0f, 359.0f) };
	const double SpawnRoll{ UKismetMathLibrary::RandomFloatInRange(0.0f, 10.0f) };

	const double SpawnScaleXY{ UKismetMathLibrary::RandomFloatInRange(1.0f, 1.2f) };
	const double SpawnScaleZ{ UKismetMathLibrary::RandomFloatInRange(2.5f, 4.0f) };

	if (!Mesh) return;

	Mesh->SetWorldRotation(FRotator{ SpawnPitch, SpawnYaw, SpawnRoll });
	Mesh->SetWorldScale3D(FVector{ SpawnScaleXY, SpawnScaleXY, SpawnScaleZ });
}


void AGrass::CreateAndAttachSpawningComponents()
{
	Rotator = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));
	Spawner = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));

	if (!Rotator || !Spawner) return;

	Rotator->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Spawner->AttachToComponent(Rotator, FAttachmentTransformRules::KeepRelativeTransform);

	AddInstanceComponent(Rotator);
	AddInstanceComponent(Spawner);
}


void AGrass::SetSpawningComponentPositions()
{
	RotatorRotation = FRotator{ 45.0, 0.0, 0.0 };

	Rotator->SetRelativeRotation(RotatorRotation);
	Rotator->SetRelativeLocation(FVector{ 0.0, 0.0, 0.0 });
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

	// put functions into a tick limiting function to slow down process + use a smaller test ground

	TryToSpawnGrass();
	UpdateRotatorRotation();
	DestroySpawningComponents();

	DrawSpawning();
}


void AGrass::UpdateRotatorRotation()
{
	if (SpawningComplete) return;

	const double RotatorPitchStart{ 45.0 };
	const double RotatorPitchEnd{ -45.0 };
	const double RotatorPitchRate{ -22.5 };
	const double RotatorYawEnd{ 300.0 };
	const double RotatorYawRate{ 60 };

	if (RotatorRotation.Pitch != RotatorPitchEnd)
	{
		RotatorRotation.Pitch += RotatorPitchRate;
	}
	else if (RotatorRotation.Yaw != RotatorYawEnd)
	{
		RotatorRotation.Yaw += RotatorYawRate;
		RotatorRotation.Pitch = RotatorPitchStart;
	}
	else
	{
		SpawningComplete = true;
		return;
	}

	Rotator->SetRelativeRotation(RotatorRotation);
}


void AGrass::TryToSpawnGrass()
{
	if (SpawningComplete) return;

	FHitResult Hit{};
	
	if (GroundHitBySpawnerRayCast(Hit)) if (!GrassHitBySpawnerSweep(Hit)) SpawnGrass(Hit);
}

bool AGrass::GroundHitBySpawnerRayCast(FHitResult& Hit)
{
	const double RayCastLength{ 3.2 };

	const FVector RayCastStart{ Spawner->GetComponentLocation() };
	const FVector RayCastEnd{ RayCastStart + (-Spawner->GetUpVector() * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


bool AGrass::GrassHitBySpawnerSweep(FHitResult& Hit)
{
	FHitResult SweepHit{};
	const FVector Impact{ Hit.ImpactPoint };
	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(1.0) }; // works at 1 but not 2 or 3?

	return GetWorld()->SweepSingleByChannel(SweepHit, Impact, Impact, FQuat::Identity, ECC_GameTraceChannel2, Sweeper);
}


void AGrass::SpawnGrass(FHitResult& Hit)
{
	const FRotator SpawnRotation{ UKismetMathLibrary::TransformRotation(RootComponent->GetComponentTransform(), RotatorRotation) };
	const FVector SpawnLocation{ Hit.ImpactPoint };

	AGrass* SpawnedGrass{ GetWorld()->SpawnActor<AGrass>(GrassClass, SpawnLocation, SpawnRotation) };
}


void AGrass::DestroySpawningComponents()
{
	if (!SpawningComplete || SpawningComponentsDestroyed) return;




	SpawningComponentsDestroyed = true;
}


void AGrass::DrawSpawning()
{
	const FVector SpawnerRayCastStart{ Spawner->GetComponentLocation() };
	const FVector SpawnerRayCastEnd{ Spawner->GetComponentLocation() + (-Spawner->GetUpVector() * 3.0) };

	DrawDebugSphere(GetWorld(), Rotator->GetComponentLocation(), 4.0f, 6, FColor::Orange);
	DrawDebugSphere(GetWorld(), Spawner->GetComponentLocation(), 1.0f, 6, FColor::Blue);
	DrawDebugLine(GetWorld(), SpawnerRayCastStart, SpawnerRayCastEnd, FColor::White);
	DrawDebugSphere(GetWorld(), SpawnerRayCastEnd, 1.0f, 6, FColor::White);
}
