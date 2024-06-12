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
	FName StaticMeshAssetLocation{ TEXT("/Game/Assets/Meshes/Grass/mowergrassv2.mowergrassv2") };

	ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(*StaticMeshAssetLocation.ToString());

	if (StaticMeshAsset.Succeeded()) StaticMesh = StaticMeshAsset.Object;
}


void AGrassD::BeginPlay()
{
	Super::BeginPlay();

	CreateAndAttachMeshComponent();
	SetYawPositions();
	SetComponentProperties();
	SetRayCastStarts();
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
	const FRotator StartingYawPosition{ 0.0, YawPositions[0], 0.0 };
	
	Root->SetRelativeRotation(StartingYawPosition);
	Root->SetMobility(EComponentMobility::Type::Static);

	Mesh->SetRelativeRotation(StartingYawPosition);
	Mesh->SetRelativeScale3D(FVector{ 1.0, 1.0, double{FMath::RandRange(1.0, 1.5)} });
	Mesh->SetMobility(EComponentMobility::Type::Static);
}


void AGrassD::SetRayCastStarts()
{
	RayCastStarts.Init(FVector::Zero(), 6);

	RootTransform = Root->GetComponentTransform();

	for (int32 Pos{ 0 }; Pos < 6; ++Pos) 
		RayCastStarts[Pos] = UKismetMathLibrary::TransformLocation(RootTransform, LocalStarts[Pos]);

	for (FVector RayCast : RayCastStarts) UE_LOG(LogTemp, Warning, TEXT("%s"), *RayCast.ToString());
}


void AGrassD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (FVector RayCast : RayCastStarts) DrawDebugSphere(GetWorld(), RayCast, 1.0f, 6, FColor::Orange);













	/*
	const FTransform Transform{ Root->GetComponentTransform() };
	const FVector Start{ Transform.GetLocation() + (Transform.GetUnitAxis(EAxis::Type::Z) * 12.0)};
	FVector Forward{ Transform.GetUnitAxis(EAxis::Type::X) };
	DrawDebugLine(GetWorld(), Start, Start + (Forward * 20.0), FColor::Orange);
	FRotator ForwardRotator{ Forward.ToOrientationRotator() };
	UE_LOG(LogTemp, Warning, TEXT("ForwardVector: %s"), *Forward.ToString());
	UE_LOG(LogTemp, Warning, TEXT("ForwardRotator: %s"), *ForwardRotator.ToString());
	ForwardRotator.Yaw += 60.0;
	*/

	// maybe my approach is to stay within local values until i need the points transformed
	// the local fvectors that represent the starting points of the raycasts can stay the same always
	// first get six circles in a hex shape around the grass

	// VectorQuaternionRotateVector
	//for (int32 I{}, double Yaw{ SpawnYaw }; I < 6; ++I, Yaw += 60.0)
	/*
	const double ArmLength{ 12.0 };
	const FTransform Transform{ Root->GetComponentTransform() };
	const FVector AngledNormal{ Transform.GetUnitAxis(EAxis::Type::Z) + Transform.GetUnitAxis(EAxis::Type::X) };
	const FVector SteepAngledNormal{ Transform.GetUnitAxis(EAxis::Type::Z) + AngledNormal };
	const FVector Start{ Transform.GetLocation() };
	const FVector End{ Start + (AngledNormal * ArmLength) };
	DrawDebugLine(GetWorld(), Start, End, FColor::Orange);
	const FVector StartHeight{ Start + (Transform.GetUnitAxis(EAxis::Type::Z) * ArmLength) };
	const FVector EndLength{ StartHeight + (Transform.GetUnitAxis(EAxis::Type::X) * ArmLength) };
	DrawDebugLine(GetWorld(), StartHeight, EndLength, FColor::Green);
	DrawDebugLine(GetWorld(), End, End + (-Transform.GetUnitAxis(EAxis::Type::Z) * (ArmLength * 2.0)), FColor::Red);
	*/

}

