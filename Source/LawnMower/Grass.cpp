// Grass actor class developed by Cody Wheeler.


#include "Grass.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"


AGrass::AGrass()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Rotator = CreateDefaultSubobject<USceneComponent>(TEXT("Rotator"));
	Spawner = CreateDefaultSubobject<USceneComponent>(TEXT("Spawner"));

	RootComponent = Root;
	Mesh->SetupAttachment(RootComponent);
	Rotator->SetupAttachment(RootComponent);
	Spawner->SetupAttachment(Rotator);

	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel3);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

	Rotator->SetRelativeLocation(FVector{ 0.0, 0.0, 3.0 });
	Rotator->SetRelativeRotation( FRotator{ 45.0, 0.0, 0.0 } );
	Spawner->SetRelativeLocation(FVector{ 6.0, 0.0, 0.0 });


}


void AGrass::BeginPlay()
{
	Super::BeginPlay();
	
	/*
	const double SpawnDepth{ -1.5 };

	const double SpawnPitch{ UKismetMathLibrary::RandomFloatInRange(0.0f, 10.0f) };
	const double SpawnYaw{ UKismetMathLibrary::RandomFloatInRange(0.0f, 359.0f) };
	const double SpawnRoll{ UKismetMathLibrary::RandomFloatInRange(0.0f, 10.0f) };
	
	const double SpawnScaleXY{ UKismetMathLibrary::RandomFloatInRange(1.0f, 1.2f) };
	const double SpawnScaleZ{ UKismetMathLibrary::RandomFloatInRange(1.25f, 1.75f) };

	FVector SpawnLocation{ Mesh->GetComponentLocation() };
	SpawnLocation.Z += SpawnDepth;

	Mesh->SetWorldLocation(SpawnLocation);
	Mesh->SetWorldRotation(FRotator{ SpawnPitch, SpawnYaw, SpawnRoll });
	Mesh->SetWorldScale3D(FVector{ SpawnScaleXY, SpawnScaleXY, SpawnScaleZ });
	*/
}


void AGrass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// get the full rotation and reset yaw and go again on repeat first: selfspawning grass stage one
	// need to set the pointers to nullptr? 6 passes? 0 > 60 > 120 > 180 > 240 > 300
	// rotate and swing using local space values and transforms to world









}
