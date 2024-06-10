// Derived AActor class AGrassC by Cody Wheeler.


#include "GrassC.h"
#include "GrassSpawnerB.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"


AGrassC::AGrassC()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignRootComponent();
	AssignStaticMesh();
}


void AGrassC::CreateAndAssignRootComponent()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	RootComponent = Root;

	Root->SetMobility(EComponentMobility::Type::Static);
}


void AGrassC::AssignStaticMesh()
{
	FName GrassTypeStandard{ TEXT("/Game/Assets/Meshes/Grass/mowergrassv2.mowergrassv2") };
	FName GrassTypeOptional{ TEXT("/Game/Assets/Meshes/Grass/mowergrassv2b.mowergrassv2b") };

	ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshStandardAsset(*GrassTypeStandard.ToString());
	ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshOptionalAsset(*GrassTypeOptional.ToString());

	if (StaticMeshStandardAsset.Succeeded()) StaticMeshStandard = StaticMeshStandardAsset.Object;
	if (StaticMeshStandardAsset.Succeeded()) StaticMeshOptional = StaticMeshOptionalAsset.Object;
}


void AGrassC::BeginPlay()
{
	Super::BeginPlay();

	RootTransform = Root->GetComponentTransform();

	CreateAndAttachRuntimeComponents();
	SetRuntimeMeshComponentProperties();
	SetRuntimeSpawningComponentProperties();
}


void AGrassC::CreateAndAttachRuntimeComponents()
{
	Mesh = Cast<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), true, GetTransform(), false));
	Rotator = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));
	Spawner = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));

	if (!Mesh || !Rotator || !Spawner) return;

	Mesh->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Rotator->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Spawner->AttachToComponent(Rotator, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	AddInstanceComponent(Mesh);
	AddInstanceComponent(Rotator);
	AddInstanceComponent(Spawner);
}


void AGrassC::SetRuntimeMeshComponentProperties()
{
	Mesh->SetStaticMesh(GetMeshType());

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AGrassC::Cut);

	const double SpawnPitchRoll{ UKismetMathLibrary::RandomFloatInRange(0.0f, 5.0f) };
	const double SpawnYaw{ UKismetMathLibrary::RandomFloatInRange(0.0f, 359.0f) };
	const double SpawnScaleZ{ UKismetMathLibrary::RandomFloatInRange(1.0f, 1.5f) };

	Mesh->SetRelativeRotation(FRotator{ SpawnPitchRoll, SpawnYaw, SpawnPitchRoll });
	Mesh->SetRelativeScale3D(FVector{ 1.0, 1.0, SpawnScaleZ });

	Mesh->SetMobility(EComponentMobility::Type::Static);

	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel3);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
}


UStaticMesh* AGrassC::GetMeshType()
{
	EGrassType GrassType{ Standard };

	if (AGrassSpawnerB* OwningSpawner{ Cast<AGrassSpawnerB>(GetOwner()) }) 
	{
		GrassType = static_cast<EGrassType>(OwningSpawner->GetGrassType());
	}

	switch (GrassType)
	{
	case Optional:	return StaticMeshOptional;
	default:		return StaticMeshStandard;
	}
}




void AGrassC::SetRuntimeSpawningComponentProperties()
{
	StartingRotatorYaw = FMath::RandRange(0, 359);
	RotatorRotation = FRotator{ 45.0, StartingRotatorYaw, 0.0 };
	Rotator->SetRelativeRotation(RotatorRotation);

	Rotator->SetRelativeLocation(FVector{ 0.0, 0.0, 3.0 });
	Spawner->SetRelativeLocation(FVector{ 15.0, 0.0, 0.0 });
}


void AGrassC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSpawnHammer();

	// TickSlowerWithDrawing();
}


void AGrassC::UpdateSpawnHammer()
{
	const double PitchMax{ 45.0 };

	TryToSpawnGrass(PitchMax);

	if (RotatorRotation.Yaw == StartingRotatorYaw + 360.0 && RotatorRotation.Pitch == -PitchMax) DestroyRuntimeSpawningComponentsAndDisableTick();
}


void AGrassC::TryToSpawnGrass(const double& PitchMax)
{
	FHitResult Hit{};

	const FVector Start{ Spawner->GetComponentLocation() };
	const FVector Direction{ -Spawner->GetUpVector() };

	const double RayCastLength{ 5.0 };

	if (RotatorRotation.Pitch == PitchMax) if (!FarDirtHitBySpawnerRayCast(Hit, Start, Direction, PitchMax)) return;

	if (RotatorRotation.Pitch == PitchMax) if (!GrowFieldOverlapped(Start)) return;

	if (GrassHitBySpawnerSweep(Start, Direction, RayCastLength, PitchMax)) return;

	if (!DirtHitBySpawnerRayCast(Hit, Start, Direction, RayCastLength, PitchMax)) return;

	SpawnGrass(Hit, PitchMax);
}


bool AGrassC::FarDirtHitBySpawnerRayCast(FHitResult& Hit, const FVector& Start, const FVector& Direction, const double& PitchMax)
{
	const FVector End{ Start + (Direction * PitchMax) };

	const bool FarGroundHit{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f);

	if (!FarGroundHit) UpdateRotatorYawAndPitch(PitchMax);

	return FarGroundHit;
}


bool AGrassC::GrowFieldOverlapped(const FVector& Start)
{
	TArray<FOverlapResult> Overlaps{};

	FCollisionObjectQueryParams GrowObjects{};
	GrowObjects.AddObjectTypesToQuery(ECC_GameTraceChannel4);

	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(1.0) };

	return GetWorld()->OverlapMultiByObjectType(Overlaps, Start, FQuat::Identity, GrowObjects, Sweeper);
}


bool AGrassC::GrassHitBySpawnerSweep(const FVector& Start, const FVector& Direction, const double& RayCastLength, const double& PitchMax)
{
	FHitResult SweepHit{};

	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(7.0) };

	const FVector End{ Start + (Direction * RayCastLength) };

	const bool GrassHit{ GetWorld()->SweepSingleByChannel(SweepHit, End, End, FQuat::Identity, ECC_GameTraceChannel2, Sweeper) };

	if (GrassHit) UpdateRotatorYawAndPitch(PitchMax);

	return GrassHit;
}


bool AGrassC::DirtHitBySpawnerRayCast(FHitResult& Hit, const FVector& Start, const FVector& Direction, const double& RayCastLength, const double& PitchMax)
{
	Hit.Reset();

	const FVector End{ Start + (Direction * RayCastLength) };

	const bool GroundHit{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	if (!GroundHit)
	{
		if (RotatorRotation.Pitch > -PitchMax) UpdateRotatorPitch(PitchMax);
		else UpdateRotatorYawAndPitch(PitchMax);
	}

	return GroundHit;
}


void AGrassC::SpawnGrass(FHitResult& Hit, const double& PitchMax)
{
	FActorSpawnParameters GrassSpawnParameters{};
	GrassSpawnParameters.Owner = GetOwner();

	const FVector SpawnLocation{ Hit.ImpactPoint };
	const FRotator SpawnRotation{ Rotator->GetComponentRotation() };

	if (AGrassC * Spawned{ GetWorld()->SpawnActor<AGrassC>(GrassClass, SpawnLocation, SpawnRotation, GrassSpawnParameters) })
	{
		if (AGrassSpawnerB * OwningSpawner{ Cast<AGrassSpawnerB>(GrassSpawnParameters.Owner) })
		{
			OwningSpawner->UpdateGrassSpawnedCount();
		}
	}

	UpdateRotatorYawAndPitch(PitchMax);
}


void AGrassC::UpdateRotatorYawAndPitch(const double& PitchMax)
{
	RotatorRotation.Yaw += 60.0;

	if (RotatorRotation.Yaw >= StartingRotatorYaw + 360.0) RotatorRotation.Pitch = -PitchMax;
	else RotatorRotation.Pitch = PitchMax;

	Rotator->SetWorldRotation(UKismetMathLibrary::TransformRotation(RootTransform, RotatorRotation));
}


void AGrassC::UpdateRotatorPitch(const double& PitchMax)
{
	if (RotatorRotation.Pitch > -PitchMax) RotatorRotation.Pitch += -22.5;

	Rotator->SetWorldRotation(UKismetMathLibrary::TransformRotation(RootTransform, RotatorRotation));
}


void AGrassC::DestroyRuntimeSpawningComponentsAndDisableTick()
{
	Spawner->DestroyComponent();
	Rotator->DestroyComponent();

	SetActorTickEnabled(false);

	// UE_LOG(LogTemp, Warning, TEXT("Spawning complete in %i ticks!"), SpawningTicks);
}


UFUNCTION() void AGrassC::Cut(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (AGrassSpawnerB * OwningSpawner{ Cast<AGrassSpawnerB>(GetOwner()) })
	{
		OwningSpawner->UpdateGrassCutCount();
	}

	Destroy();
}


void AGrassC::TickSlowerWithDrawing()
{
	++TickCount;

	if (TickCount >= 15.0f)
	{
		UpdateSpawnHammer();

		DrawSpawningComponents();

		++SpawningTicks;

		TickCount = 0.0f;
	}
}


void AGrassC::DrawSpawningComponents()
{
	const FVector Start{ Spawner->GetComponentLocation() };
	const FVector Direction{ -Spawner->GetUpVector() };
	const FVector GroundEnd{ Start + (Direction * 5.0) };

	DrawDebugSphere(GetWorld(), Start, 1.0f, 6, FColor::Blue, false, 5.0f);
	DrawDebugLine(GetWorld(), Start, GroundEnd, FColor::White, false, 5.0f);
	DrawDebugSphere(GetWorld(), GroundEnd, 3.0f, 6, FColor::Green, false, 5.0f);
}
