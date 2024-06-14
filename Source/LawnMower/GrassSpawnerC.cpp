// Derived AActor class AGrassSpawnerC by Cody Wheeler.


#include "GrassSpawnerC.h"

#include "Components/BoxComponent.h"
#include "GrassD.h"
#include "GrassPoolA.h"
#include "Kismet/GameplayStatics.h"
// #include "LawnMowerGameMode.h"


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
	Collider->SetBoxExtent(FVector{ 200.0, 200.0, 200.0 });

	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerC::ActivateSpawner);
}


UFUNCTION() void AGrassSpawnerC::ActivateSpawner(
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


void AGrassSpawnerC::BeginPlay()
{
	Super::BeginPlay();

	SetGrassPoolOwner();
}


void AGrassSpawnerC::SetGrassPoolOwner()
{
	TArray<AActor*> GrassPool{};

	UGameplayStatics::GetAllActorsOfClassWithTag(this, AGrassPoolA::StaticClass(), FName{ TEXT("Pool") }, GrassPool);

	SetOwner(GrassPool[0]);
}


void AGrassSpawnerC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SpawnedGrassCleared())
	{
		UpdateGameMode();

		ActivateActorByTag();

		SetActorTickEnabled(false);
	}
}


void AGrassSpawnerC::TryToSpawnGrass()
{
	FHitResult Hit{};

	if (RayCastHitGround(Hit)) SpawnGrass(Hit);
}


bool AGrassSpawnerC::RayCastHitGround(FHitResult& Hit) const
{
	const FTransform ActorWorldTransform{ GetActorTransform() };

	const FVector Start{ ActorWorldTransform.GetLocation() };
	const FVector End{ Start + (-ActorWorldTransform.GetUnitAxis(EAxis::Type::Z) * 25.0) };

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1);
}


void AGrassSpawnerC::SpawnGrass(const FHitResult& Hit)
{
	const FVector Location{ Hit.ImpactPoint };
	const FRotator Rotation{ RootComponent->GetComponentRotation() };

	FActorSpawnParameters SpawnedOwner{};
	SpawnedOwner.Owner = this;

	if (AGrassD * Spawned{ GetWorld()->SpawnActor<AGrassD>(AGrassD::StaticClass(), Location, Rotation, SpawnedOwner) })
	{
		++GrassSpawnedCount;

		bSpawnSuccessful = true;
	}
}


bool AGrassSpawnerC::SpawnedGrassCleared() const
{
	if (!bSpawnSuccessful) return false;

	UE_LOG(LogTemp, Warning, TEXT("Spawned %d, Cut %d"), GrassSpawnedCount, GrassCutCount);

	return GrassSpawnedCount == GrassCutCount;
}


void AGrassSpawnerC::UpdateGameMode() const
{
	/*
	if (ALawnMowerGameMode * GameMode{ Cast<ALawnMowerGameMode>(GetWorld()->GetAuthGameMode()) })
	{
		GameMode->UpdateSpawnersCleared();
	}
	*/
}


void AGrassSpawnerC::ActivateActorByTag()
{

}


void AGrassSpawnerC::UpdateGrassSpawnedCount() { ++GrassSpawnedCount; }
void AGrassSpawnerC::UpdateGrassCutCount() { ++GrassCutCount; };
