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
	
	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UAudioComponent* SongLoop {};

	const float FadeInSongLoopCountMaximum{ 1.0f };
	const float FadeInSongLoopCountRate{ 0.25f };

public:
	AMowerGameModeA();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void FadeInSongLoop(const float DeltaTime);

private:

	float FadeInSongLoopCount{};
	bool bFadeInSongLoopCountMaxReached{};

};
