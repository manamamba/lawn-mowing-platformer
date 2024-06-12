// Derived AActor class AGrassPoolA by Cody Wheeler.


#include "GrassPoolA.h"

#include "GrassD.h"


AGrassPoolA::AGrassPoolA()
{
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(TEXT("Pool"));
}


void AGrassPoolA::BeginPlay()
{
	Super::BeginPlay();
}


void AGrassPoolA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

