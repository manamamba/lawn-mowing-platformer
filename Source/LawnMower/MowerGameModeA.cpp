// Derived AGameMode class AMowerGameModeA by Cody Wheeler.


#include"MowerGameModeA.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"


AMowerGameModeA::AMowerGameModeA()
{
	SongLoop = CreateDefaultSubobject<UAudioComponent>(TEXT("SongLoop"));

	SongLoop->SetupAttachment(RootComponent);
}


void AMowerGameModeA::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(true);
}


void AMowerGameModeA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}