// Derived AActor class APlanetoidA by Cody Wheeler.


#include "PlanetoidA.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MowerB.h"


APlanetoidA::APlanetoidA()
{
	PrimaryActorTick.bCanEverTick = true;

	SetComponentProperties();
}

void APlanetoidA::SetComponentProperties()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));

	RootComponent = Root;

	Mesh->SetupAttachment(RootComponent);
	Collider->SetupAttachment(RootComponent);

	Root->SetMobility(EComponentMobility::Type::Static);

	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	Mesh->SetMobility(EComponentMobility::Type::Static);

	Collider->SetCollisionProfileName(TEXT("Custom..."));
	Collider->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	Collider->SetCollisionObjectType(ECC_PhysicsBody);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	Collider->SetSphereRadius(150.0f);
	Collider->SetMobility(EComponentMobility::Type::Static);
}


void APlanetoidA::BeginPlay()
{
	Super::BeginPlay();

	SetMowerProperties();

	Collider->OnComponentBeginOverlap.AddDynamic(this, &APlanetoidA::Capture);
	Collider->OnComponentEndOverlap.AddDynamic(this, &APlanetoidA::Release);
}

void APlanetoidA::SetMowerProperties()
{
	Mower = Cast<AMowerB>(UGameplayStatics::GetPlayerPawn(this, 0));

	if (!Mower) return;

	MowerPhysicsBody = Mower->GetPhysicsBody();

	MowerMass = Mower->GetPhysicsBodyMass();

	if (AntiGravityMultiplier) PlanetoidAcceleration = GravitationalAcceleration / abs(AntiGravityMultiplier);
	else PlanetoidAcceleration = GravitationalAcceleration;

	MowerCaptureForce = MowerMass * PlanetoidAcceleration;
}

void APlanetoidA::Capture(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	MowerCaptured = true;
}

void APlanetoidA::Release(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	MowerCaptured = false;
}


void APlanetoidA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MowerCaptured) ApplyCaptureForce(DeltaTime);
}

void APlanetoidA::ApplyCaptureForce(const float DeltaTime)
{
	if (!Mower || !MowerPhysicsBody) return;

	Mower->Float();

	const FVector PullDirection{ FVector{ GetActorLocation() - Mower->GetActorLocation() }.GetSafeNormal() };

	MowerPhysicsBody->AddForce(PullDirection * MowerCaptureForce);
}
