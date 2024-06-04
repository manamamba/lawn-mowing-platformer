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
	SetRuntimeMeshComponentProperties();
	SetRuntimeSpawningComponentProperties();
}


void AGrassB::CreateAndAttachRuntimeComponents()
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


void AGrassB::SetRuntimeMeshComponentProperties()
{
	Mesh->SetStaticMesh(StaticMesh);
	
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


void AGrassB::SetRuntimeSpawningComponentProperties()
{
	RotatorRotation = FRotator{ 45.0, 0.0, 0.0 };

	Rotator->SetRelativeRotation(RotatorRotation);

	Rotator->SetRelativeLocation(FVector{ 0.0, 0.0, 3.0 });
	Spawner->SetRelativeLocation(FVector{ 12.0f, 0.0, 0.0 });
}


void AGrassB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryToSpawnGrass();

	if (RotatorRotation.Yaw == 360.0 && RotatorRotation.Pitch == -45.0) DestroyRuntimeSpawningComponentsAndDisableTick();

	// TickSlowerWithDrawing();
}


void AGrassB::TryToSpawnGrass()
{
	FHitResult Hit{};

	const FVector Start{ Spawner->GetComponentLocation() };
	const FVector Direction{ -Spawner->GetUpVector() };

	if (RotatorRotation.Pitch == 45.0) if (!FarGroundHitBySpawnerRayCast(Hit, Start, Direction)) return;

	if (GrassHitBySpawnerSweep(Start, Direction)) return;

	if (!GroundHitBySpawnerRayCast(Hit, Start, Direction)) return;

	SpawnGrass(Hit);
}


bool AGrassB::FarGroundHitBySpawnerRayCast(FHitResult& Hit, const FVector& Start, const FVector& Direction)
{
	const FVector End{ Start + (Direction * 35.0) };

	const bool FarGroundHit{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f);

	if (!FarGroundHit) UpdateRotatorYawAndPitch();

	return FarGroundHit;
}


bool AGrassB::GrassHitBySpawnerSweep(const FVector& Start, const FVector& Direction)
{
	FHitResult SweepHit{};

	const FCollisionShape Sweeper{ FCollisionShape::MakeSphere(3.0) };
	
	const FVector End{ Start + (Direction * 5.0) };

	const bool GrassHit{ GetWorld()->SweepSingleByChannel(SweepHit, End, End, FQuat::Identity, ECC_GameTraceChannel2, Sweeper) };

	if (GrassHit) UpdateRotatorYawAndPitch();

	return GrassHit;
}


bool AGrassB::GroundHitBySpawnerRayCast(FHitResult& Hit, const FVector& Start, const FVector& Direction)
{
	Hit.Reset();

	const FVector End{ Start + (Direction * 5.0) };

	const bool GroundHit{ GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1) };

	if (!GroundHit)
	{
		if (RotatorRotation.Pitch > -45.0) UpdateRotatorPitch();
		else UpdateRotatorYawAndPitch();
	}

	return GroundHit;
}


void AGrassB::SpawnGrass(FHitResult& Hit)
{
	AGrassB* SpawnedGrass{ GetWorld()->SpawnActor<AGrassB>(GrassBClass, Hit.ImpactPoint, Rotator->GetComponentRotation()) };

	UpdateRotatorYawAndPitch();
}


void AGrassB::UpdateRotatorYawAndPitch()
{
	RotatorRotation.Yaw += 60.0;

	if (RotatorRotation.Yaw >= 360.0) RotatorRotation.Pitch = -45.0;
	else RotatorRotation.Pitch = 45.0;
	
	Rotator->SetWorldRotation(UKismetMathLibrary::TransformRotation(RootTransform, RotatorRotation));
}


void AGrassB::UpdateRotatorPitch()
{
	if (RotatorRotation.Pitch > -45.0) RotatorRotation.Pitch += -22.5;

	Rotator->SetWorldRotation(UKismetMathLibrary::TransformRotation(RootTransform, RotatorRotation));
}


void AGrassB::DestroyRuntimeSpawningComponentsAndDisableTick()
{
	Spawner->DestroyComponent();
	Rotator->DestroyComponent();

	SetActorTickEnabled(false);

	// UE_LOG(LogTemp, Warning, TEXT("Spawning complete in %i ticks!"), SpawningCompleteTicks);
}


void AGrassB::TickSlowerWithDrawing()
{
	++TickCount;

	if (TickCount >= 60.0f)
	{
		TryToSpawnGrass();

		DrawSpawningComponents();

		if (RotatorRotation.Yaw == 360.0 && RotatorRotation.Pitch == -45.0) DestroyRuntimeSpawningComponentsAndDisableTick();

		++SpawningCompleteTicks;

		TickCount = 0.0f;
	}
}


void AGrassB::DrawSpawningComponents()
{
	const FVector Start{ Spawner->GetComponentLocation() };
	const FVector Direction{ -Spawner->GetUpVector() };
	const FVector FarGroundEnd{ Start + (Direction * 35.0) };
	const FVector GroundEnd{ Start + (Direction * 5.0) };

	DrawDebugSphere(GetWorld(), Start, 1.0f, 6, FColor::Blue, false, 5.0f);
	DrawDebugLine(GetWorld(), Start, GroundEnd, FColor::White, false, 5.0f);
	DrawDebugSphere(GetWorld(), GroundEnd, 3.0f, 6, FColor::Green, false, 5.0f);
}
