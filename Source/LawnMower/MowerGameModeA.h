// Derived AGameMode class AMowerGameModeA by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MowerGameModeA.generated.h"

class UAudioComponent;


UCLASS()
class LAWNMOWER_API AMowerGameModeA : public AGameMode
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = Component) UAudioComponent* SongLoop {};

public:
	AMowerGameModeA();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:

};
