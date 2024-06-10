// Derived AActor class AGrassSpawnerV1 by Cody Wheeler.


#include "GrassSpawnerV1.h"
#include "GrassV1.h"


AGrassSpawnerV1::AGrassSpawnerV1()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
}


void AGrassSpawnerV1::BeginPlay()
{
	Super::BeginPlay();
}


void AGrassSpawnerV1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult LineHit{};

	const FTransform SpawnerWorldTransform{ Mesh->GetComponentTransform() };

	const FVector SpawnerLocation{ SpawnerWorldTransform.GetLocation() };
	const FVector SpawnerDownVector{ -SpawnerWorldTransform.GetUnitAxis(EAxis::Type::Z) };
	const double RayCastLength{ 25.0 };
	const FVector RayCastEnd{ SpawnerLocation + (SpawnerDownVector * RayCastLength) };

	bool bGroundHit{ GetWorld()->LineTraceSingleByChannel(LineHit, SpawnerLocation, RayCastEnd, ECC_GameTraceChannel1) };

	if (bGroundHit)
	{
		FHitResult SweepHit{};
		const FVector Start{ LineHit.ImpactPoint };
		const FVector End{ LineHit.ImpactPoint };
		const FCollisionShape GrassSweeper{ FCollisionShape::MakeSphere(3.0) };

		const bool GrassHit{ GetWorld()->SweepSingleByChannel(SweepHit, Start, End, FQuat::Identity, ECC_GameTraceChannel2, GrassSweeper) };

		if (GrassHit) DrawDebugSphere(GetWorld(), LineHit.ImpactPoint, 3.0f, 12, FColor::Yellow);

		if (!GrassHit)
		{
			const FRotator SpawnerRotation{ Mesh->GetComponentRotation() };
			const FVector SpawnLocation{ LineHit.ImpactPoint };

			AGrassV1* SpawnedGrass{ GetWorld()->SpawnActor<AGrassV1>(GrassClass, SpawnLocation, SpawnerRotation) };

			if (SpawnedGrass)
			{
				SpawnedGrass->SetOwner(this);
				UE_LOG(LogTemp, Warning, TEXT("GrassSpawned %i"), ++GrassSpawned);
			}
		}
	}

	if (bGroundHit)
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

