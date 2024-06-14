// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatformA.h"


AMovingPlatformA::AMovingPlatformA()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Root;

	Mesh->SetupAttachment(RootComponent);

	Mesh->SetCollisionProfileName(TEXT("Custom..."));
	Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
}


void AMovingPlatformA::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);
}


void AMovingPlatformA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UE_LOG(LogTemp, Warning, TEXT("%s Activated!"), *GetName());

	SetActorTickEnabled(false);
}
