// Derived AGameModeBase class ALawnMowerGameMode by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LawnMowerGameMode.generated.h"


UCLASS()
class LAWNMOWER_API ALawnMowerGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public: 
	void UpdateGrassCut(int32 Grass);
	int32 GetGrassCut() const;

private:
	int32 GrassCut{};

};
