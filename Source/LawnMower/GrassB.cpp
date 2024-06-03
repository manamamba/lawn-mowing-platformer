// Fill out your copyright notice in the Description page of Project Settings.


#include "GrassB.h"
#include "Kismet/KismetMathLibrary.h"

AGrassB::AGrassB()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateAndAssignRootComponent();
	AssignStaticMesh();
}


void AGrassB::CreateAndAssignRootComponent()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	RootComponent = Root;

	Root->SetMobility(EComponentMobility::Type::Static);
}


void AGrassB::AssignStaticMesh()
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(TEXT("/Game/Assets/Meshes/mowergrassv2.mowergrassv2"));

	if (StaticMeshAsset.Succeeded()) StaticMesh = StaticMeshAsset.Object;
}


void AGrassB::BeginPlay()
{
	Super::BeginPlay();

	RootTransform = Root->GetComponentTransform();

	CreateAndAttachRuntimeComponents();
}


void AGrassB::CreateAndAttachRuntimeComponents()
{
	Mesh = Cast<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), true, GetTransform(), false));
	Rotator = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));
	Spawner = Cast<USceneComponent>(AddComponentByClass(USceneComponent::StaticClass(), true, GetTransform(), false));

	if (!Mesh || !Rotator || !Spawner) return;

	Mesh->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);		// research these rules!
	Rotator->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Spawner->AttachToComponent(Rotator, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	if (StaticMesh) Mesh->SetStaticMesh(StaticMesh);

	SetRuntimeComponentProperties();

	AddInstanceComponent(Mesh);
	AddInstanceComponent(Rotator);
	AddInstanceComponent(Spawner);
}


void AGrassB::SetRuntimeComponentProperties()
{
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
	
	RotatorRotation = FRotator{ 45.0, 0.0, 0.0 };

	Rotator->SetRelativeRotation(RotatorRotation);

	Rotator->SetRelativeLocation(FVector{ 0.0, 0.0, 3.0 });
	Spawner->SetRelativeLocation(FVector{ 9.0f, 0.0, 0.0 });
}


void AGrassB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryToSpawnGrass();

	// TickSlowerWithDrawing();
}


void AGrassB::TryToSpawnGrass()
{
	FHitResult Hit{};

	if (GroundHitBySpawnerRayCast(Hit)) if (!GrassHitBySpawnerSweep(Hit)) SpawnGrass(Hit);
}


bool AGrassB::GroundHitBySpawnerRayCast(FHitResult& Hit)
{
	const FVector RayCastStart{ Spawner->GetComponentLocation() };
	const FVector RayCastDirection{ -Spawner->GetUpVector() };

	bool bEdgeNotHit{ true };

	double RayCastLength{ 50.0 };

	FVector RayCastEnd{ RayCastStart + (RayCastDirection * RayCastLength) };

	if (RotatorRotation.Pitch == 45.0)
	{
		bEdgeNotHit = GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1);

		// DrawDebugLine(GetWorld(), RayCastStart, RayCastEnd, FColor::Red, false, 5.0f);
		// DrawDebugSphere(GetWorld(), RayCastEnd, 1.0f, 6, FColor::Red, false, 5.0f);
	}

	if (!bEdgeNotHit)
	{
		ResetSpawningHammer();
		return false;
	}

	Hit.Reset();

	RayCastEnd = RayCastStart + (RayCastDirection * (RayCastLength = 5.0));

	const bool GrassHit{ GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel2) };

	// DrawDebugLine(GetWorld(), RayCastStart, RayCastEnd, FColor::Green, false, 5.0f);
	// DrawDebugSphere(GetWorld(), RayCastEnd, 1.0f, 6, FColor::Green, false, 5.0f);

	if (GrassHit)
	{
		ResetSpawningHammer();
		return false;
	}

	Hit.Reset();

	RayCastEnd = RayCastStart + (RayCastDirection * (RayCastLength = 3.1));

	const bool GroundHit{ GetWorld()->LineTraceSingleByChannel(Hit, RayCastStart, RayCastEnd, ECC_GameTraceChannel1) };

	if (!GroundHit) UpdateRotatorRotation();

	return GroundHit;
}


bool AGrassB::GrassHitBySpawnerSweep(FHitResult& Hit)
{
	FHitResult SweepHit{};
	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(4.0) };

	const bool GrassHit{ GetWorld()->SweepSingleByChannel(SweepHit, Hit.ImpactPoint, Hit.ImpactPoint, FQuat::Identity, ECC_GameTraceChannel2, Sweeper) };

	if (GrassHit) UpdateRotatorRotation();

	return GrassHit;
}


void AGrassB::SpawnGrass(FHitResult& Hit)
{
	AGrassB* SpawnedGrass{ GetWorld()->SpawnActor<AGrassB>(GrassBClass, Hit.ImpactPoint, Rotator->GetComponentRotation()) };

	ResetSpawningHammer();
}


void AGrassB::UpdateRotatorRotation()
{
	if (RotatorRotation.Pitch > -45.0) RotatorRotation.Pitch += -22.5;

	if (DestroyRuntimeComponentsAndDisableTick()) return;

	Rotator->SetWorldRotation(UKismetMathLibrary::TransformRotation(RootTransform, RotatorRotation));
}


void AGrassB::ResetSpawningHammer()
{
	RotatorRotation.Yaw += 60.0;

	if (RotatorRotation.Yaw >= 360.0) RotatorRotation.Pitch = -45.0;
	else RotatorRotation.Pitch = 45.0;

	if (DestroyRuntimeComponentsAndDisableTick()) return;

	Rotator->SetWorldRotation(UKismetMathLibrary::TransformRotation(RootTransform, RotatorRotation));
}


bool AGrassB::DestroyRuntimeComponentsAndDisableTick()
{
	if (RotatorRotation.Yaw != 360.0 && RotatorRotation.Pitch != -45.0) return false;
	
	Spawner->DestroyComponent();
	Rotator->DestroyComponent();

	SetActorTickEnabled(false);

	UE_LOG(LogTemp, Warning, TEXT("Spawning Complete In %i Ticks!"), SpawningCompleteTicks);

	return true;
}


void AGrassB::TickSlowerWithDrawing()
{
	++TickCount;

	if (TickCount >= 60.0f)
	{
		TryToSpawnGrass();
		DrawSpawningComponents();

		++SpawningCompleteTicks;

		TickCount = 0.0f;
	}
}


void AGrassB::DrawSpawningComponents()
{
	const FVector SpawnerRayCastStart{ Spawner->GetComponentLocation() };
	const FVector SpawnerRayCastEnd{ Spawner->GetComponentLocation() + (-Spawner->GetUpVector() * 3.1) };

	DrawDebugSphere(GetWorld(), Rotator->GetComponentLocation(), 4.0f, 6, FColor::Orange);
	DrawDebugSphere(GetWorld(), Spawner->GetComponentLocation(), 1.0f, 6, FColor::Blue, false, 3.0f);
	DrawDebugLine(GetWorld(), SpawnerRayCastStart, SpawnerRayCastEnd, FColor::White, false, 3.0f);
	DrawDebugSphere(GetWorld(), SpawnerRayCastEnd, 1.0f, 6, FColor::White, false, 3.0f);
}
