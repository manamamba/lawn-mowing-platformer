// GrassSpawnerC actor class developed by Cody Wheeler.


#include "GrassSpawnerC.h"
#include "Components/BoxComponent.h"
#include "GrassC.h"
#include "LawnMowerGameMode.h"


AGrassSpawnerC::AGrassSpawnerC()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootProperties();
	SetColliderProperties();
}


void AGrassSpawnerC::SetRootProperties()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	RootComponent = Root;
}


void AGrassSpawnerC::SetColliderProperties()
{
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

	SetupOverlapDelegate();
}


void AGrassSpawnerC::SetupOverlapDelegate() //try in constructor
{
	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerC::ActivateSpawner);
}


void AGrassSpawnerC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Spawn();

	if (SpawnedGrassCleared()) DisableSpawnerTick();
}


void AGrassSpawnerC::Spawn()
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
			UpdateGrassSpawnedCount();

			bSpawnSuccessful = true;
			bSpawnerActivated = false;
		}
	}
}


bool AGrassSpawnerC::SpawnedGrassCleared()
{
	if (!bSpawnSuccessful || bSpawnerActivated) return false;

	return 	GrassSpawnedCount == GrassCutCount;
}


void AGrassSpawnerC::DisableSpawnerTick() { SetActorTickEnabled(false); }


int32 AGrassSpawnerC::GetGrassType() const { return GrassType; }
int32 AGrassSpawnerC::GetGrassSpawned() const { return GrassSpawnedCount; };
int32 AGrassSpawnerC::GetGrassCut() const { return GrassCutCount; };


void AGrassSpawnerC::UpdateGrassSpawnedCount() { ++GrassSpawnedCount; }
void AGrassSpawnerC::UpdateGrassCutCount() { ++GrassCutCount; };


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
}
