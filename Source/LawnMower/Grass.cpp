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
	Mesh->SetRelativeLocation(FVector{ 0.0, 0.0, 6.0 });
	Mesh->SetRelativeScale3D(FVector{ 1.0, 1.0, 2.5 });
}


void AGrass::BeginPlay()
{
	Super::BeginPlay();

	RootTransform = Root->GetComponentTransform();

	RandomizeRotationAndScale();
	CreateAndAttachSpawningComponents();
	// SetSpawningComponentsRotationAndLocation();
}


void AGrass::RandomizeRotationAndScale()
{
	if (!Mesh) return;
	
	const double SpawnPitchRoll{ UKismetMathLibrary::RandomFloatInRange(0.0f, 5.0f) };
	const double SpawnYaw{ UKismetMathLibrary::RandomFloatInRange(0.0f, 359.0f) };
	const double SpawnScaleZ{ UKismetMathLibrary::RandomFloatInRange(2.5f, 3.5f) };

	Mesh->SetRelativeRotation(FRotator{ SpawnPitchRoll, SpawnYaw, SpawnPitchRoll });
	Mesh->SetRelativeScale3D(FVector{ 1.0, 1.0, SpawnScaleZ });
}


void AGrass::CreateAndAttachSpawningComponents()
{
	Rotator = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));
	Spawner = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));

	if (!Rotator || !Spawner) return;

	Rotator->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Spawner->AttachToComponent(Rotator, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	SetSpawningComponentsRotationAndLocation();

	Rotator->SetMobility(EComponentMobility::Type::Movable);
	Spawner->SetMobility(EComponentMobility::Type::Movable);

	AddInstanceComponent(Rotator);
	AddInstanceComponent(Spawner);
}


void AGrass::SetSpawningComponentsRotationAndLocation()
{
	RotatorRotation = FRotator{ 67.5, 0.0, 0.0 };

	Rotator->SetRelativeRotation(RotatorRotation);	
	Rotator->SetRelativeLocation(FVector{ 0.0, 0.0, 3.0 });
	Spawner->SetRelativeLocation(FVector{ 9.0f, 0.0, 0.0 }); // grass cannot be touching raised from 7
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

	// TickSlowerWithDrawing();

	TryToSpawnGrass();
	UpdateRotatorRotation();
}


void AGrass::TickSlowerWithDrawing()
{
	++TickCount;

	if (TickCount >= 30.0f)
	{
		TryToSpawnGrass();
		DrawSpawningComponents();
		UpdateRotatorRotation();

		// ++SpawningCompleteTicks;

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
	double RayCastLength{ 5.0 };

	const FVector RayCastStart{ Spawner->GetComponentLocation() };
	const FVector SpawnerDownVector{ -Spawner->GetUpVector() };

	FVector RayCastEnd{ RayCastStart + (SpawnerDownVector * RayCastLength) };

	const bool GrassWasHit{ GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel2) };

	if (GrassWasHit)
	{
		ResetSpawningHammer();
		return false;
	}

	Hit.Reset();

	RayCastLength = 3.1;

	RayCastEnd =  RayCastStart + SpawnerDownVector * RayCastLength;

	return GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);
}


bool AGrass::GrassHitBySpawnerSweep(FHitResult& Hit) const
{
	FHitResult SweepHit{};
	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(4.0) }; // increased so grass wont touch

	return GetWorld()->SweepSingleByChannel(SweepHit, Hit.ImpactPoint, Hit.ImpactPoint, FQuat::Identity, ECC_GameTraceChannel2, Sweeper);
}


void AGrass::SpawnGrass(FHitResult& Hit)
{
	AGrass* SpawnedGrass{ GetWorld()->SpawnActor<AGrass>(GrassClass, Hit.ImpactPoint, Rotator->GetComponentRotation()) };

	ResetSpawningHammer();
}


void AGrass::DrawSpawningComponents()
{
	const FVector SpawnerRayCastStart{ Spawner->GetComponentLocation() };
	const FVector SpawnerRayCastEnd{ Spawner->GetComponentLocation() + (-Spawner->GetUpVector() * 3.1) };

	// DrawDebugSphere(GetWorld(), Rotator->GetComponentLocation(), 4.0f, 6, FColor::Orange);
	DrawDebugSphere(GetWorld(), Spawner->GetComponentLocation(), 1.0f, 6, FColor::Blue);
	DrawDebugLine(GetWorld(), SpawnerRayCastStart, SpawnerRayCastEnd, FColor::White);
	DrawDebugSphere(GetWorld(), SpawnerRayCastEnd, 1.0f, 6, FColor::White);
}


void AGrass::UpdateRotatorRotation()
{
	if (RotatorRotation.Pitch > -67.5) RotatorRotation.Pitch += -22.5;
	else if (RotatorRotation.Yaw < 300.0) ResetSpawningHammer();
	else
	{
		DestroySpawningComponentsAndDisableTick();
		return;
	}

	Rotator->SetWorldRotation(UKismetMathLibrary::TransformRotation(RootTransform, RotatorRotation));
}


void AGrass::ResetSpawningHammer()
{
	RotatorRotation.Yaw += 60.0;
	
	if (RotatorRotation.Yaw >= 360.0 ) RotatorRotation.Pitch = -67.5;
	else RotatorRotation.Pitch = 67.5;
}


void AGrass::DestroySpawningComponentsAndDisableTick()
{
	Spawner->DestroyComponent();
	Rotator->DestroyComponent();

	SetActorTickEnabled(false);

	// UE_LOG(LogTemp, Warning, TEXT("Spawning Complete In %i Ticks!"), SpawningCompleteTicks);
}
