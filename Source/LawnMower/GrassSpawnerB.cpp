// Derived AActor class AGrassSpawnerB by Cody Wheeler.


#include "GrassSpawnerB.h"
#include "Components/BoxComponent.h"
#include "GrassC.h"
#include "LawnMowerGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"


AGrassSpawnerB::AGrassSpawnerB()
{
	PrimaryActorTick.bCanEverTick = true;

	SetComponentProperties();
}


void AGrassSpawnerB::SetComponentProperties()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));

	RootComponent = Root;

	Collider->SetupAttachment(RootComponent);

	Collider->SetCollisionProfileName(TEXT("Custom..."));
	Collider->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Collider->SetCollisionObjectType(ECC_GameTraceChannel3);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	Collider->SetBoxExtent(FVector{ 200.0, 200.0, 200.0 });

	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerB::ActivateSpawner);
}


void AGrassSpawnerB::BeginPlay()
{
	Super::BeginPlay();
}


void AGrassSpawnerB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryToSpawnGrass();

	if (SpawnedGrassCleared())
	{
		UpdateGameModeGrassCut();
		ActivateActorByTag();
		DisableSpawnerTick();
	}
}


void AGrassSpawnerB::TryToSpawnGrass()
{
	if (bSpawnSuccessful || !bSpawnerActivated) return;

	FHitResult Hit{};

	if (SpawnerHitGround(Hit)) SpawnGrass(Hit);
}


bool AGrassSpawnerB::SpawnerHitGround(FHitResult& Hit) const
{
	const double TraceLength{ 25.0 };

	const FTransform ActorWorldTransform{ GetActorTransform() };

	const FVector Start{ ActorWorldTransform.GetLocation() };
	const FVector End{ Start + (-ActorWorldTransform.GetUnitAxis(EAxis::Type::Z) * TraceLength) };

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1);
}


void AGrassSpawnerB::SpawnGrass(const FHitResult& Hit)
{
	const FVector SpawnLocation{ Hit.ImpactPoint };
	const FRotator SpawnRotation{ RootComponent->GetComponentRotation() };

	FActorSpawnParameters SpawnOwner{};
	SpawnOwner.Owner = this;

	if (AGrassC * Spawned{ GetWorld()->SpawnActor<AGrassC>(GrassClass, SpawnLocation, SpawnRotation, SpawnOwner) })
	{
		UpdateGrassSpawnedCount();

		bSpawnSuccessful = true;
		bSpawnerActivated = false;
	}
}


bool AGrassSpawnerB::SpawnedGrassCleared() const
{
	if (!bSpawnSuccessful || bSpawnerActivated) return false;

	return GrassSpawnedCount == GrassCutCount;
}


void AGrassSpawnerB::UpdateGameModeGrassCut() const
{
	if (ALawnMowerGameMode * GameMode{ Cast<ALawnMowerGameMode>(GetWorld()->GetAuthGameMode()) })
	{
		GameMode->UpdateGrassCut(GrassCutCount);
	}
}


void AGrassSpawnerB::ActivateActorByTag()
{
	// TArray<AActor*> OutActors{};

	// UGameplayStatics::GetAllActorsOfClassWithTag(this, AStaticMeshActor::StaticClass(), ActorTag, OutActors);

	// for (AActor* Actor : OutActors) ...

}


void AGrassSpawnerB::DisableSpawnerTick() { SetActorTickEnabled(false); }


int32 AGrassSpawnerB::GetGrassType() const { return GrassType; }
int32 AGrassSpawnerB::GetGrassSpawned() const { return GrassSpawnedCount; };
int32 AGrassSpawnerB::GetGrassCut() const { return GrassCutCount; };


void AGrassSpawnerB::UpdateGrassSpawnedCount() { ++GrassSpawnedCount; }
void AGrassSpawnerB::UpdateGrassCutCount() { ++GrassCutCount; };


UFUNCTION() void AGrassSpawnerB::ActivateSpawner(
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
