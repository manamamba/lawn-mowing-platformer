// Derived AGameModeBase class ALawnMowerGameMode by Cody Wheeler.


#include "LawnMowerGameMode.h"


void ALawnMowerGameMode::UpdateSpawnersCleared() 
{ 
	++SpawnersCleared;

	UE_LOG(LogTemp, Warning, TEXT("Spawners Cleared: %d"), SpawnersCleared);
}

int32 ALawnMowerGameMode::GetSpawnersCleared() const { return SpawnersCleared; };

