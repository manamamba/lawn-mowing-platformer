// GrassSpawnerC actor class developed by Cody Wheeler.


#include "GrassSpawnerC.h"
#include "GrassC.h"


AGrassSpawnerC::AGrassSpawnerC()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AGrassSpawnerC::BeginPlay()
{
	Super::BeginPlay();
}


void AGrassSpawnerC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// future: set an early return for a spawner that should wait to spawn later in the level
	// perhaps give it a collider (or another object that is one) that once an overlap event occurs, tick turns on
	// after spawning wait for spawned grass == destroyed grass to trigger something

	TryToSpawn();
}


void AGrassSpawnerC::TryToSpawn()
{
	if (bSpawnSuccessful) return;
	
	FHitResult Hit{};

	const FTransform ActorWorldTransform{ GetActorTransform() };
	const FVector Start{ ActorWorldTransform.GetLocation() };
	const FVector End{ Start + (-ActorWorldTransform.GetUnitAxis(EAxis::Type::Z) * 25.0) };

	const bool GroundHit{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1)};

	if (GroundHit)
	{
		FActorSpawnParameters GrassSpawnParameters{};
		GrassSpawnParameters.Owner = this;

		AGrassC* SpawnedGrass{ GetWorld()->SpawnActor<AGrassC>(GrassClassC, Hit.ImpactPoint, RootComponent->GetComponentRotation(), GrassSpawnParameters)};

		if (SpawnedGrass)
		{
			// SpawnedGrass->SetOwner(this);

			// DisableSpawnerTick();

			bSpawnSuccessful = true;
		}
	}

	if (GroundHit)
	{
		DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Green);
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 1.0f, 6, FColor::Green);
	}
	else
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red);
		DrawDebugSphere(GetWorld(), End, 1.0f, 6, FColor::Red);
	}
}


void AGrassSpawnerC::DisableSpawnerTick() { SetActorTickEnabled(false); }


int32 AGrassSpawnerC::GetGrassType() const { return GrassType; }


void AGrassSpawnerC::UpdateGrassSpawnedCount() { ++GrassSpawnedCount; }
