// Derived AActor class AGrassSpawnerV2 by Cody Wheeler.


#include "GrassSpawnerV2.h"
#include "Components/BoxComponent.h"
#include "GrassV3.h"
#include "LawnMowerGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"


AGrassSpawnerV2::AGrassSpawnerV2()
{
	PrimaryActorTick.bCanEverTick = true;

	SetComponentProperties();
}


void AGrassSpawnerV2::SetComponentProperties()
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

	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerV2::ActivateSpawner);
}


void AGrassSpawnerV2::BeginPlay()
{
	Super::BeginPlay();
}


void AGrassSpawnerV2::Tick(float DeltaTime)
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


void AGrassSpawnerV2::TryToSpawnGrass()
{
	if (bSpawnSuccessful || !bSpawnerActivated) return;

	FHitResult Hit{};

	if (SpawnerHitGround(Hit)) SpawnGrass(Hit);
}


bool AGrassSpawnerV2::SpawnerHitGround(FHitResult& Hit) const
{
	const double TraceLength{ 25.0 };

	const FTransform ActorWorldTransform{ GetActorTransform() };

	const FVector Start{ ActorWorldTransform.GetLocation() };
	const FVector End{ Start + (-ActorWorldTransform.GetUnitAxis(EAxis::Type::Z) * TraceLength) };

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1);
}


void AGrassSpawnerV2::SpawnGrass(const FHitResult& Hit)
{
	const FVector SpawnLocation{ Hit.ImpactPoint };
	const FRotator SpawnRotation{ RootComponent->GetComponentRotation() };

	FActorSpawnParameters SpawnOwner{};
	SpawnOwner.Owner = this;

	if (AGrassV3 * Spawned{ GetWorld()->SpawnActor<AGrassV3>(GrassClass, SpawnLocation, SpawnRotation, SpawnOwner) })
	{
		UpdateGrassSpawnedCount();

		bSpawnSuccessful = true;
		bSpawnerActivated = false;
	}
}


bool AGrassSpawnerV2::SpawnedGrassCleared() const
{
	if (!bSpawnSuccessful || bSpawnerActivated) return false;

	return GrassSpawnedCount == GrassCutCount;
}


void AGrassSpawnerV2::UpdateGameModeGrassCut() const
{
	if (ALawnMowerGameMode * GameMode{ Cast<ALawnMowerGameMode>(GetWorld()->GetAuthGameMode()) })
	{
		GameMode->UpdateGrassCut(GrassCutCount);
	}
}


void AGrassSpawnerV2::ActivateActorByTag()
{
	// TArray<AActor*> OutActors{};

	// UGameplayStatics::GetAllActorsOfClassWithTag(this, AStaticMeshActor::StaticClass(), ActorTag, OutActors);

	// for (AActor* Actor : OutActors) ...

}


void AGrassSpawnerV2::DisableSpawnerTick() { SetActorTickEnabled(false); }


int32 AGrassSpawnerV2::GetGrassType() const { return GrassType; }
int32 AGrassSpawnerV2::GetGrassSpawned() const { return GrassSpawnedCount; };
int32 AGrassSpawnerV2::GetGrassCut() const { return GrassCutCount; };


void AGrassSpawnerV2::UpdateGrassSpawnedCount() { ++GrassSpawnedCount; }
void AGrassSpawnerV2::UpdateGrassCutCount() { ++GrassCutCount; };


UFUNCTION() void AGrassSpawnerV2::ActivateSpawner(
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
