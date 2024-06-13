// Derived AActor class AGrassSpawnerD by Cody Wheeler.


#include "GrassSpawnerD.h"

#include "Components/BoxComponent.h"
#include "GrassE.h"
#include "Kismet/GameplayStatics.h"
#include "LawnMowerGameMode.h"
#include "MowerB.h"


AGrassSpawnerD::AGrassSpawnerD()
{
	PrimaryActorTick.bCanEverTick = true;

	SetComponentProperties();
}

void AGrassSpawnerD::SetComponentProperties()
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

	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerD::ActivateSpawner);
}

UFUNCTION() void AGrassSpawnerD::ActivateSpawner(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	TryToSpawnGrass();

	Collider->DestroyComponent();
}


void AGrassSpawnerD::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);
}


void AGrassSpawnerD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SpawnedGrassCleared())
	{
		UpdateGameMode();

		if (bSetNewMowerSpawn) UpdateMowerRespawn();

		if (bActivateActorByTag) ActivateActorByTag();

		SetActorTickEnabled(false);
	}
}

void AGrassSpawnerD::TryToSpawnGrass()
{
	FHitResult Hit{};

	if (RayCastHitGround(Hit)) SpawnGrass(Hit);
}

bool AGrassSpawnerD::RayCastHitGround(FHitResult& Hit) const
{
	const FTransform ActorWorldTransform{ GetActorTransform() };

	const FVector Start{ ActorWorldTransform.GetLocation() };
	const FVector End{ Start + (-ActorWorldTransform.GetUnitAxis(EAxis::Type::Z) * 25.0) };

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1);
}

void AGrassSpawnerD::SpawnGrass(const FHitResult& Hit)
{
	const FVector Location{ Hit.ImpactPoint };
	const FRotator Rotation{ RootComponent->GetComponentRotation() };

	FActorSpawnParameters SpawnedOwner{};
	SpawnedOwner.Owner = this;

	if (AGrassE* NewGrass{ GetWorld()->SpawnActor<AGrassE>(AGrassE::StaticClass(), Location, Rotation, SpawnedOwner) })
	{
		GrassChildrenActivelySpawning.Add(NewGrass);

		++GrassSpawnedCount;

		SetActorTickEnabled(true);
	}
}

bool AGrassSpawnerD::SpawnedGrassCleared() const
{
	return GrassSpawnedCount == GrassCutCount;
}

void AGrassSpawnerD::UpdateGameMode() const
{
	if (ALawnMowerGameMode * GameMode{ Cast<ALawnMowerGameMode>(GetWorld()->GetAuthGameMode()) })
	{
		GameMode->UpdateSpawnersCleared();
	}
}

void AGrassSpawnerD::UpdateMowerRespawn()
{
	AMowerB* Mower{ Cast<AMowerB>(UGameplayStatics::GetPlayerPawn(this, 0)) };





}

void AGrassSpawnerD::ActivateActorByTag()
{

	// UGameplayStatics::GetAllActorsOfClassWithTag

}

void AGrassSpawnerD::UpdateGrassSpawnedCount() { ++GrassSpawnedCount; }
void AGrassSpawnerD::UpdateGrassCutCount() { ++GrassCutCount; };
