// Derived AActor class AGrassSpawnerD by Cody Wheeler.


#include "GrassSpawnerD.h"

#include "Components/BoxComponent.h"
#include "GameFramework/GameMode.h"
#include "GrassE.h"
#include "Kismet/GameplayStatics.h"
#include "MovingPlatformA.h"
#include "MowerB.h"
#include "MowerGameModeA.h"


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
	Root->SetMobility(EComponentMobility::Type::Static);

	Collider->SetupAttachment(RootComponent);
	Collider->SetMobility(EComponentMobility::Type::Static);

	Collider->SetCollisionProfileName(TEXT("Custom..."));
	Collider->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Collider->SetCollisionObjectType(ECC_PhysicsBody);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	Collider->SetBoxExtent(FVector{ 200.0, 200.0, 200.0 });

	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerD::ActivateSpawner);
}

void AGrassSpawnerD::ActivateSpawner(
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
		if (bActivatePlatformByTag) ActivateActorByTag();

		if (bSetNewMowerSpawn) UpdateMowerRespawn();

		if (bRestartGame) RestartGame();

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
		++GrassSpawnedCount;
		++GrassActivelySpawning;

		SetActorTickEnabled(true);
	}
}

bool AGrassSpawnerD::SpawnedGrassCleared() const
{
	return GrassSpawnedCount == GrassCutCount;
}

void AGrassSpawnerD::ActivateActorByTag()
{
	TArray<AActor*> Platforms{};
	
	UGameplayStatics::GetAllActorsOfClassWithTag(this, AMovingPlatformA::StaticClass(), PlatformTag, Platforms);

	if (Platforms.Num() == 0) return;

	if (AMovingPlatformA * Platform{ Cast<AMovingPlatformA>(Platforms[0]) })
	{
		Platform->SetActorTickEnabled(true);
	}
}

void AGrassSpawnerD::UpdateMowerRespawn()
{
	AMowerB* Mower{ Cast<AMowerB>(UGameplayStatics::GetPlayerPawn(this, 0)) };

	const FVector NewLocation{ GetActorLocation() + (Root->GetUpVector() * 500.0)};

	Mower->SetNewRespawnLocation(NewLocation);
}

void AGrassSpawnerD::RestartGame()
{
	if (AMowerGameModeA * GameMode{ Cast<AMowerGameModeA>(UGameplayStatics::GetGameMode(this)) })
	{
		GameMode->RestartGame();
	}
}

void AGrassSpawnerD::UpdateGrassSpawnedCount() { ++GrassSpawnedCount; }
void AGrassSpawnerD::UpdateGrassCutCount() { ++GrassCutCount; };
void AGrassSpawnerD::IncreaseGrassActivelySpawning() { ++GrassActivelySpawning; }
void AGrassSpawnerD::DecreaseGrassActivelySpawning() { --GrassActivelySpawning; }
int32 AGrassSpawnerD::GetGrassActivelySpawning() const { return GrassActivelySpawning; }
