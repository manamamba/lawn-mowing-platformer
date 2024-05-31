// Grass spawner actor class developed by Cody Wheeler.


#include "GrassSpawner.h"


AGrassSpawner::AGrassSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AGrassSpawner::BeginPlay()
{
	Super::BeginPlay();
}


void AGrassSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

