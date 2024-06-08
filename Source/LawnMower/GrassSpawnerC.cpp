// GrassSpawnerC actor class developed by Cody Wheeler.


#include "GrassSpawnerC.h"
#include "Components/BoxComponent.h"
#include "GrassC.h"
#include "LawnMowerGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

AGrassSpawnerC::AGrassSpawnerC()
{
	PrimaryActorTick.bCanEverTick = true;

	SetComponentProperties();
}


void AGrassSpawnerC::SetComponentProperties()
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

	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerC::ActivateSpawner);
}


void AGrassSpawnerC::BeginPlay()
{
	Super::BeginPlay();
}


void AGrassSpawnerC::Tick(float DeltaTime)
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


void AGrassSpawnerC::TryToSpawnGrass()
{
	if (bSpawnSuccessful || !bSpawnerActivated) return;
	
	FHitResult Hit{};

	if (SpawnerHitGround(Hit)) SpawnGrass(Hit);
}


bool AGrassSpawnerC::SpawnerHitGround(FHitResult& Hit) const
{
	const double TraceLength{ 25.0 };

	const FTransform ActorWorldTransform{ GetActorTransform() };

	const FVector Start{ ActorWorldTransform.GetLocation() };
	const FVector End{ Start + (-ActorWorldTransform.GetUnitAxis(EAxis::Type::Z) * TraceLength) };

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1);
}


void AGrassSpawnerC::SpawnGrass(const FHitResult& Hit)
{
	const FVector SpawnLocation{ Hit.ImpactPoint };
	const FRotator SpawnRotation{ RootComponent->GetComponentRotation() };

	FActorSpawnParameters SpawnOwner{};
	SpawnOwner.Owner = this;

	if (AGrassC* Spawned{ GetWorld()->SpawnActor<AGrassC>(GrassClassC, SpawnLocation, SpawnRotation, SpawnOwner) })
	{
		UpdateGrassSpawnedCount();

		bSpawnSuccessful = true;
		bSpawnerActivated = false;
	}
}


bool AGrassSpawnerC::SpawnedGrassCleared() const
{
	if (!bSpawnSuccessful || bSpawnerActivated) return false;

	return GrassSpawnedCount == GrassCutCount;
}


void AGrassSpawnerC::UpdateGameModeGrassCut() const
{
	if (ALawnMowerGameMode * GameMode{ Cast<ALawnMowerGameMode>(GetWorld()->GetAuthGameMode()) })
	{
		GameMode->UpdateGrassCut(GrassCutCount);
	}
}


void AGrassSpawnerC::ActivateActorByTag()
{
	// TArray<AActor*> OutActors{};
	
	// UGameplayStatics::GetAllActorsOfClassWithTag(this, AStaticMeshActor::StaticClass(), ActorTag, OutActors);

	// for (AActor* Actor : OutActors) ...

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
