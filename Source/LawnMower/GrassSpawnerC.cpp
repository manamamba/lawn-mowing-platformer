// Derived AActor class AGrassSpawnerB by Cody Wheeler.


#include "GrassSpawnerC.h"

#include "Components/BoxComponent.h"
#include "GrassD.h"
#include "GrassPoolA.h"
#include "Kismet/GameplayStatics.h"


AGrassSpawnerC::AGrassSpawnerC()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetComponentProperties();
}


void AGrassSpawnerC::SetComponentProperties()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));

	RootComponent = Root;

	Collider->SetupAttachment(RootComponent);

	Collider->SetCollisionProfileName(TEXT("Custom..."));
	Collider->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Collider->SetCollisionObjectType(ECC_GameTraceChannel3);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	Collider->SetBoxExtent(FVector{ 200.0, 200.0, 200.0 });

	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGrassSpawnerC::ActivateSpawner);
}


void AGrassSpawnerC::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> OutActors{};

	const FName PoolTag{ TEXT("Pool") };

	UGameplayStatics::GetAllActorsOfClassWithTag(this, AGrassPoolA::StaticClass(), PoolTag, OutActors);
	
	for (AActor* Actor : OutActors) UE_LOG(LogTemp, Warning, TEXT("Actor Found by Spawner: %s"), *Actor->GetName());
}


void AGrassSpawnerC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


UFUNCTION() void AGrassSpawnerC::ActivateSpawner(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	bSpawnerActivated = true;

	Collider->DestroyComponent();
}
