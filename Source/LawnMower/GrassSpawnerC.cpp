// GrassSpawnerC actor class developed by Cody Wheeler.


#include "GrassSpawnerC.h"
#include "Components/BoxComponent.h"
#include "GrassC.h"


AGrassSpawnerC::AGrassSpawnerC()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	RootComponent = Root;

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));

	Collider->SetupAttachment(RootComponent);

	Collider->SetCollisionProfileName(TEXT("Custom..."));
	Collider->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Collider->SetCollisionObjectType(ECC_GameTraceChannel3);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
}


void AGrassSpawnerC::BeginPlay()
{
	Super::BeginPlay();

	if (Collider) Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerC::ActivateSpawner);
}


void AGrassSpawnerC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryToSpawn();
}


void AGrassSpawnerC::TryToSpawn()
{
	if (bSpawnSuccessful || !bSpawnerActivated) return;
	
	FHitResult Hit{};

	const FTransform ActorWorldTransform{ GetActorTransform() };
	const FVector Start{ ActorWorldTransform.GetLocation() };
	const FVector End{ Start + (-ActorWorldTransform.GetUnitAxis(EAxis::Type::Z) * 25.0) };

	const bool GroundHit{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1)};

	if (GroundHit)
	{
		FActorSpawnParameters SpawnParameters{};
		SpawnParameters.Owner = this;

		const FVector SpawnLocation{ Hit.ImpactPoint };
		const FRotator SpawnRotation{ RootComponent->GetComponentRotation() };

		AGrassC* SpawnedGrass{ GetWorld()->SpawnActor<AGrassC>(GrassClassC, SpawnLocation, SpawnRotation, SpawnParameters)};

		if (SpawnedGrass)
		{
			// SpawnedGrass->SetOwner(this);

			// DisableSpawnerTick();

			bSpawnSuccessful = true;
			bSpawnerActivated = false;
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


UFUNCTION() void AGrassSpawnerC::ActivateSpawner(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	bSpawnerActivated = true;

	Collider->DestroyComponent();

	UE_LOG(LogTemp, Warning, TEXT("Overlapped!"));
}
