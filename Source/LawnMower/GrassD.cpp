// Derived AActor class AGrassD by Cody Wheeler.


#include "GrassD.h"

#include "GrassPoolA.h"
#include "GrassSpawnerC.h"
#include "Kismet/KismetMathLibrary.h"


AGrassD::AGrassD()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignRootComponent();
	LocateAndAssignStaticMesh();
}


void AGrassD::CreateAndAssignRootComponent()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	RootComponent = Root;

	Root->SetMobility(EComponentMobility::Type::Static);
}


void AGrassD::LocateAndAssignStaticMesh()
{
	FName StaticMeshAssetLocation{ TEXT("/Game/Assets/Meshes/Grass/mowergrassv2.mowergrassv2") };

	ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(*StaticMeshAssetLocation.ToString());

	if (StaticMeshAsset.Succeeded()) StaticMesh = StaticMeshAsset.Object;
}


void AGrassD::BeginPlay()
{
	Super::BeginPlay();

	SpawnOwner = Cast<AGrassSpawnerC>(GetOwner());
	PoolOwner = Cast<AGrassPoolA>(SpawnOwner->GetOwner());

	CreateAndAttachMeshComponent();
	SetMeshComponentProperties();
}


void AGrassD::CreateAndAttachMeshComponent()
{
	Mesh = Cast<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), true, GetTransform(), false));

	if (!Mesh) return;

	Mesh->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	AddInstanceComponent(Mesh);

	Mesh->SetStaticMesh(StaticMesh);
}


void AGrassD::SetMeshComponentProperties()
{
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AGrassD::Cut);

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


UFUNCTION() void AGrassD::Cut(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bCut) return;

	if (SpawnOwner)
	{
		SpawnOwner->UpdateGrassCutCount();
		
		if (PoolOwner)
		{
			bCut = true;
			
			Root->SetMobility(EComponentMobility::Type::Movable);
			Mesh->SetMobility(EComponentMobility::Type::Movable);
			Mesh->SetVisibility(false);

			SetActorLocation(FVector::Zero());

			PoolOwner->PooledGrass.Add(this);

			UE_LOG(LogTemp, Warning, TEXT("Grass cut! Cut Pool Size %d"), PoolOwner->PooledGrass.Num());
		}
	}
	
	// Destroy();
}


void AGrassD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// double TickTime{ FPlatformTime::Seconds() };
	
	TickCount += DeltaTime;

	if (TickCount >= TickCountMax)
	{
		TryToSpawnGrass();

		TickCount = 0.0f;
	}

	// UE_LOG(LogTemp, Warning, TEXT("TickTime %f"), TickTime = FPlatformTime::Seconds() - TickTime);
}


void AGrassD::TryToSpawnGrass()
{
	if (SpawnAttempts == 0)
	{
		RootTransform = Root->GetComponentTransform();
		RootDownVector = -RootTransform.GetUnitAxis(EAxis::Type::Z);
	}

	if (SpawnAttempts == 6)
	{
		SetActorTickEnabled(false);

		SpawnAttempts = 0;

		return;
	}

	const int32 SpawnPosition{ SpawnAttempts++ };

	FHitResult Hit{};

	const FVector RayCastStart{ UKismetMathLibrary::TransformLocation(RootTransform, LocalRayCastStarts[SpawnPosition]) };

	if (!RayCastStartInsideGrowField(RayCastStart)) return;
	if (!RayCastHitGround(Hit, RayCastStart, RootDownVector)) return;
	if (GrassNearRayCastImpact(Hit.ImpactPoint)) return;

	SpawnGrass(Hit.ImpactPoint, GetSpawnRotation(RootTransform, Hit.Time, YawRotations[SpawnPosition]));
}


bool AGrassD::RayCastStartInsideGrowField(const FVector& RayCastStart)
{
	TArray<FOverlapResult> Overlaps{};

	FCollisionObjectQueryParams GrowObjects{};
	GrowObjects.AddObjectTypesToQuery(ECC_GameTraceChannel4);

	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(1.0) };

	return GetWorld()->OverlapMultiByObjectType(Overlaps, RayCastStart, FQuat::Identity, GrowObjects, Sweeper);
}


bool AGrassD::RayCastHitGround(FHitResult& Hit, const FVector& RayCastStart, const FVector& RayCastDirection)
{
	const FVector End{ RayCastStart + (RayCastDirection * RayCastLength) };

	return GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, End, ECC_GameTraceChannel1);
}


bool AGrassD::GrassNearRayCastImpact(const FVector& Impact)
{
	FHitResult SweepHit{};

	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(FMath::RandRange(9.0, 9.5)) };

	return GetWorld()->SweepSingleByChannel(SweepHit, Impact, Impact, FQuat::Identity, ECC_GameTraceChannel2, Sweeper);
}


FRotator AGrassD::GetSpawnRotation(const FTransform& Transform, const float& TraceLength, const double& YawPosition)
{
	const FRotator RelativeRotation{ 45.0 - (90.0 * TraceLength), YawPosition, 0.0 };
	
	return UKismetMathLibrary::TransformRotation(Transform, RelativeRotation);
}


void AGrassD::SpawnGrass(const FVector& Location, const FRotator& Rotation)
{
	FActorSpawnParameters SpawnedOwner{};
	SpawnedOwner.Owner = GetOwner();

	AGrassD* NewGrass{};
	
	if (PoolOwner && PoolOwner->PooledGrass.Num() != 0)
	{
		NewGrass = PoolOwner->PooledGrass.Pop();

		if (!NewGrass) return;

		NewGrass->SetOwner(SpawnedOwner.Owner);
		NewGrass->SpawnOwner = Cast<AGrassSpawnerC>(SpawnedOwner.Owner);
		NewGrass->SetActorLocation(Location);
		NewGrass->SetActorRotation(Rotation);
		NewGrass->Root->SetMobility(EComponentMobility::Type::Static);
		NewGrass->Mesh->SetMobility(EComponentMobility::Type::Static);
		NewGrass->Mesh->SetVisibility(true);
		NewGrass->SetActorTickEnabled(true);
		NewGrass->bCut = false;

		UE_LOG(LogTemp, Warning, TEXT("Grass pulled! Cut Pool Size %d"), PoolOwner->PooledGrass.Num());
	}
	else
	{
		NewGrass = GetWorld()->SpawnActor<AGrassD>(AGrassD::StaticClass(), Location, Rotation, SpawnedOwner);
	}

	if (SpawnOwner) SpawnOwner->UpdateGrassSpawnedCount();
}
