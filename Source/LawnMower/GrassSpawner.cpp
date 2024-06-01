// GrassSpawner actor class developed by Cody Wheeler.


#include "GrassSpawner.h"


AGrassSpawner::AGrassSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
}


void AGrassSpawner::BeginPlay()
{
	Super::BeginPlay();
}


void AGrassSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult LineHit{};

	const FTransform SpawnerWorldTransform{ Mesh->GetComponentTransform() };

	const FVector SpawnerLocation{ SpawnerWorldTransform.GetLocation() };
	const FVector SpawnerDownVector{ -SpawnerWorldTransform.GetUnitAxis(EAxis::Type::Z) };
	const double RayCastLength{ 100.0 };
	const FVector RayCastEnd{ SpawnerLocation + (SpawnerDownVector * RayCastLength) };

	bool GroundHit{ GetWorld()->LineTraceSingleByChannel(LineHit, SpawnerLocation, RayCastEnd, ECC_GameTraceChannel1) };

	if (GroundHit)
	{	
		DrawDebugSphere(GetWorld(), LineHit.ImpactPoint, 3.0f, 12, FColor::Orange);

		FHitResult SweepHit{};
		const FVector Start{ LineHit.ImpactPoint };
		const FVector End{ LineHit.ImpactPoint };
		const FCollisionShape GrassSweeper{ FCollisionShape::MakeSphere(3.0) };

		const bool GrassHit{ GetWorld()->SweepSingleByChannel(SweepHit, Start, End, FQuat::Identity, ECC_GameTraceChannel2, GrassSweeper) };

		if (GrassHit) DrawDebugSphere(GetWorld(), LineHit.ImpactPoint, 3.0f, 12, FColor::Yellow);
	}

	if (GroundHit)
	{
		DrawDebugLine(GetWorld(), SpawnerLocation, LineHit.ImpactPoint, FColor::Green);
		DrawDebugSphere(GetWorld(), LineHit.ImpactPoint, 1.0f, 6, FColor::Green);
	}
	else
	{
		DrawDebugLine(GetWorld(), SpawnerLocation, RayCastEnd, FColor::Red);
		DrawDebugSphere(GetWorld(), RayCastEnd, 1.0f, 6, FColor::Red);
	}
}

