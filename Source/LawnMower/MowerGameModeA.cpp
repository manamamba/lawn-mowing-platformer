// Derived AGameMode class AMowerGameModeA by Cody Wheeler.


#include "MowerGameModeA.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"


AMowerGameModeA::AMowerGameModeA()
{
	SongLoop = CreateDefaultSubobject<UAudioComponent>(TEXT("SongLoop"));

	SongLoop->SetupAttachment(RootComponent);

	SongLoop->VolumeMultiplier = 0.0f;
}


void AMowerGameModeA::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(true);
}


void AMowerGameModeA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FadeInSongLoop(DeltaTime);
}

void AMowerGameModeA::FadeInSongLoop(const float DeltaTime)
{
	if (bFadeInSongLoopCountMaxReached) return;

	FadeInSongLoopCount += FadeInSongLoopCountRate * DeltaTime;

	if (FadeInSongLoopCount >= FadeInSongLoopCountMaximum)
	{
		FadeInSongLoopCount = FadeInSongLoopCountMaximum;
		bFadeInSongLoopCountMaxReached = true;
	}

	SongLoop->SetVolumeMultiplier(FadeInSongLoopCount);
}