// Derived AActor class AGrassE by Cody Wheeler.


#include "GrassE.h"

#include "GrassSpawnerD.h"
#include "Kismet/KismetMathLibrary.h"


AGrassE::AGrassE()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignRootComponent();
	LocateAndAssignStaticMesh();
}

void AGrassE::CreateAndAssignRootComponent()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	RootComponent = Root;

	Root->SetMobility(EComponentMobility::Type::Static);
}

void AGrassE::LocateAndAssignStaticMesh()
{
	const FName StaticMeshAssetLocation{ TEXT("/Game/Assets/Meshes/Grass/mowergrassv2.mowergrassv2") };

	ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(*StaticMeshAssetLocation.ToString());

	if (StaticMeshAsset.Succeeded()) StaticMesh = StaticMeshAsset.Object;
}


void AGrassE::BeginPlay()
{
	Super::BeginPlay();

	CreateAndAttachMeshComponent();
	SetMeshComponentProperties();

	SpawnOwner = Cast<AGrassSpawnerD>(GetOwner());
	RootTransform = Root->GetComponentTransform();
	RootDownVector = -RootTransform.GetUnitAxis(EAxis::Type::Z);
}

void AGrassE::CreateAndAttachMeshComponent()
{
	Mesh = Cast<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), true, GetTransform(), false));

	if (!Mesh) return;

	Mesh->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	AddInstanceComponent(Mesh);

	Mesh->SetStaticMesh(StaticMesh);
}

void AGrassE::SetMeshComponentProperties()
{
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AGrassE::Cut);

	const double Yaw{ static_cast<double>(FMath::RandRange(0, 359)) };
	const double PitchRoll{ FMath::RandRange(0.0, 5.0) };
	const double ScaleZ{ FMath::RandRange(1.0, 1.5) };

	Mesh->SetRelativeRotation(FRotator{ PitchRoll, Yaw, PitchRoll });
	Mesh->SetRelativeScale3D(FVector{ 1.0, 1.0, ScaleZ });

	Mesh->SetMobility(EComponentMobility::Type::Static);

	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel3);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
}

void AGrassE::Cut(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!SpawnOwner) return;

	SpawnOwner->UpdateGrassCutCount();

	if (SpawnAttempts != 6) SpawnOwner->DecreaseGrassActivelySpawning();

	Destroy();
}


void AGrassE::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ReadyToTrySpawning(DeltaTime)) TrySpawningGrass();
}

bool AGrassE::ReadyToTrySpawning(const float& DeltaTime)
{
	if (!SpawnOwner) return false;

	if (SpawnOwner->GetGrassActivelySpawning() > 50 && SpawnAttempts == 0) TickCountThreshold = 0.16f;

	TickCount += DeltaTime;

	if (TickCount < TickCountThreshold) return false;
	else TickCount = 0.0f;

	if (SpawnAttempts > 5)
	{
		SpawnOwner->DecreaseGrassActivelySpawning();

		SetActorTickEnabled(false);

		return false;
	}

	return true;
}

void AGrassE::TrySpawningGrass()
{
	const int32 SpawnPosition{ SpawnAttempts++ };

	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(RootTransform, LocalRayCastStarts[SpawnPosition]) };

	FHitResult Hit{};

	if (!RayCastHitGround(Hit, RayCastStart, RootDownVector)) return;
	if (!SafeToSpawnGrass(Hit.ImpactPoint)) return;

	SpawnGrass(Hit.ImpactPoint, GetSpawnRotation(RootTransform, Hit.Time, YawRotations[SpawnPosition]));
}

bool AGrassE::RayCastHitGround(FHitResult& Hit, const FVector& RayCastStart, const FVector& RayCastDirection)
{
	const FVector End{ RayCastStart + (RayCastDirection * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, End, ECC_GameTraceChannel1);
}

bool AGrassE::SafeToSpawnGrass(FVector& ImpactPoint)
{
	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(9.0) };

	TArray<FHitResult> Hits{};

	FCollisionObjectQueryParams Types{};
	Types.AddObjectTypesToQuery(ECC_GameTraceChannel3);
	Types.AddObjectTypesToQuery(ECC_GameTraceChannel4);

	if (!GetWorld()->SweepMultiByObjectType(Hits, ImpactPoint, ImpactPoint, FQuat::Identity, Types, Sweeper)) return false;

	bool bGrassOverlap{}, bGrowOverlap{};

	for (int32 I{}; I < Hits.Num(); ++I)
	{
		switch (Hits[I].Component->GetCollisionObjectType())
		{
		case ECC_GameTraceChannel3: bGrassOverlap = true;		break;
		case ECC_GameTraceChannel4: bGrowOverlap = true;
		}
	}

	if (bGrowOverlap && !bGrassOverlap) return true;
	else return false;
}

FRotator AGrassE::GetSpawnRotation(const FTransform& Transform, const float TraceLength, const double YawPosition)
{
	const FRotator RelativeRotation{ 45.0 - (90.0 * TraceLength), YawPosition, 0.0 };

	return UKismetMathLibrary::TransformRotation(Transform, RelativeRotation);
}

void AGrassE::SpawnGrass(const FVector& Location, const FRotator& Rotation)
{
	FActorSpawnParameters SpawnedOwner{};
	SpawnedOwner.Owner = GetOwner();

	if (AGrassE * NewGrass{ GetWorld()->SpawnActor<AGrassE>(AGrassE::StaticClass(), Location, Rotation, SpawnedOwner) })
	{
		if (!SpawnOwner) return;

		SpawnOwner->IncreaseGrassActivelySpawning();
		SpawnOwner->UpdateGrassSpawnedCount();
	}
}
