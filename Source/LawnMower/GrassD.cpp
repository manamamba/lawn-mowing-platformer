// Derived AActor class AGrassD by Cody Wheeler.


#include "GrassD.h"

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

	Root->SetMobility(EComponentMobility::Type::Movable);
}


void AGrassD::LocateAndAssignStaticMesh()
{
	if (StaticMesh) return;

	FName StaticMeshAssetLocation{ TEXT("/Game/Assets/Meshes/Grass/mowergrassv2.mowergrassv2") };

	ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(*StaticMeshAssetLocation.ToString());

	if (StaticMeshAsset.Succeeded()) StaticMesh = StaticMeshAsset.Object;

	// UE_LOG(LogTemp, Warning, TEXT("Mesh Set!")); // need to check this again when spawner is ready
}


void AGrassD::BeginPlay()
{
	Super::BeginPlay();

	CreateAndAttachMeshComponent();
	SetYawPositions();
	SetComponentProperties();
	// SetRayCastStarts();
}


void AGrassD::CreateAndAttachMeshComponent()
{
	Mesh = Cast<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), true, GetTransform(), false));

	if (!Mesh) return;

	Mesh->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	AddInstanceComponent(Mesh);

	Mesh->SetStaticMesh(StaticMesh);
}


void AGrassD::SetYawPositions()
{
	YawPositions.Init(0.0, 6);

	YawPositions[0] = FMath::RandRange(0, 359);

	for (int32 Pos{ 1 }; Pos < 6; ++Pos) YawPositions[Pos] = YawPositions[Pos - 1] + 60.0;
}


void AGrassD::SetComponentProperties()
{
	//Root->SetRelativeRotation(FRotator{ 0.0, YawPositions[0], 0.0 });
	Root->SetMobility(EComponentMobility::Type::Static);

	// Mesh->SetRelativeRotation(FRotator{ 0.0, YawPositions[0], 0.0 });
	Mesh->SetRelativeScale3D(FVector{ 1.0, 1.0, double{FMath::RandRange(1.0, 1.5)} });
	Mesh->SetMobility(EComponentMobility::Type::Static);

	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel3);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

	RootTransform = Root->GetComponentTransform();
	RootDownVector = -RootTransform.GetUnitAxis(EAxis::Type::Z);
}


void AGrassD::SetRayCastStarts()
{
	RayCastStarts.Init(FVector::Zero(), 6);

	for (int32 Start{ 0 }; Start < 6; ++Start)
		RayCastStarts[Start] = UKismetMathLibrary::TransformLocation(RootTransform, LocalRayCastStarts[Start]);
}


void AGrassD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// double TickTime{ FPlatformTime::Seconds() };
	

	if (SpawnPosition == 0)
	{
		RootRotation = Root->GetComponentRotation();

		SetYawPositions();
		SetRayCastStarts();
	}

	if (SpawnPosition == 6)
	{
		SetActorTickEnabled(false);
		return;
	}

	TickCount += DeltaTime;

	if (TickCount >= TickCountMax)
	{
		TryToSpawnGrass();

		++SpawnPosition;

		TickCount = 0.0f;
	}


	// UE_LOG(LogTemp, Warning, TEXT("TickTime %f"), TickTime = FPlatformTime::Seconds() - TickTime);
}


void AGrassD::TryToSpawnGrass()
{
	FHitResult Hit{};

	if (!RayCastStartInsideGrowField(RayCastStarts[SpawnPosition])) return;
	if (!RayCastHitGround(Hit, RayCastStarts[SpawnPosition], RootDownVector)) return;
	if (GrassNearRayCastImpact(Hit.ImpactPoint)) return;

		DrawDebugSphere(GetWorld(), RayCastStarts[SpawnPosition], 4.0f, 6, FColor::Magenta, false, 3.0f);
		DrawDebugLine(GetWorld(), RayCastStarts[SpawnPosition], Hit.ImpactPoint, FColor::Magenta, false, 3.0f);
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 4.0f, 6, FColor::Magenta, false, 3.0f);

	SpawnGrass(Hit.ImpactPoint, GetSpawnRotation(Hit.Time, YawPositions[SpawnPosition]));
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

	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(7.0) };

	return GetWorld()->SweepSingleByChannel(SweepHit, Impact, Impact, FQuat::Identity, ECC_GameTraceChannel2, Sweeper);
}


FRotator AGrassD::GetSpawnRotation(const float& TraceLength, const double& YawPosition)
{
	const FRotator RelativeRotation{ 45.0 - (90.0 * TraceLength), YawPosition, 0.0 };
	
	return UKismetMathLibrary::TransformRotation(RootTransform, RelativeRotation);
}


void AGrassD::SpawnGrass(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	FActorSpawnParameters GrassSpawnParameters{};
	GrassSpawnParameters.Owner = this;

	if (AGrassD * NewGrass{ GetWorld()->SpawnActor<AGrassD>(AGrassD::StaticClass(), SpawnLocation, SpawnRotation, GrassSpawnParameters) })
	{
		// NewGrass->Root->SetMobility(EComponentMobility::Type::Movable);
		// NewGrass->Root->SetRelativeRotation(NewSpawnRelativeRotation);
	}
}

// use this grass blade's worldtransform, new local
// set up the newspawanrotation, after passing all checks and before spawning the new wgrass
// set relative rotation after in spawn? start with tick off? set new relative rotation, turn tick on?
// or grass gets info from owner before changing to static


