// Derived AActor class AGrassPoolA by Cody Wheeler.


#include "GrassPoolA.h"


AGrassPoolA::AGrassPoolA()
{
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(TEXT("Pool"));
}


void AGrassPoolA::BeginPlay()
{
	Super::BeginPlay();

	for (FName Tag : Tags) UE_LOG(LogTemp, Warning, TEXT("GrassPoolTag: %s"), *Tag.ToString());
}


void AGrassPoolA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

