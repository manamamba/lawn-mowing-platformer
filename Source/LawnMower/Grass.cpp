// Grass actor class developed by Cody Wheeler.


#include "Grass.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"


AGrass::AGrass()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Mesh;

	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel3);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
}


void AGrass::BeginPlay()
{
	Super::BeginPlay();

	const double SpawnPitch{ UKismetMathLibrary::RandomFloatInRange(0.0f, 10.0f) };
	const double SpawnYaw{ UKismetMathLibrary::RandomFloatInRange(0.0f, 359.0f) };
	const double SpawnRoll{ UKismetMathLibrary::RandomFloatInRange(0.0f, 10.0f) };

	const double SpawnScaleXY{ UKismetMathLibrary::RandomFloatInRange(1.0f, 1.2f) };
	const double SpawnScaleZ{ UKismetMathLibrary::RandomFloatInRange(1.25f, 1.75f) };

	double SpawnDepth{ 1.5 };

	FVector WorldLocation{ Mesh->GetComponentLocation() };
	WorldLocation.Z -= SpawnDepth;

	Mesh->SetWorldLocation(WorldLocation);
	Mesh->SetWorldRotation(FRotator{ SpawnPitch, SpawnYaw, SpawnRoll });
	Mesh->SetWorldScale3D(FVector{ SpawnScaleXY, SpawnScaleXY, SpawnScaleZ });
}


void AGrass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
