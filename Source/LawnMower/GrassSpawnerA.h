// Derived AActor class AGrassSpawnerA by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawnerA.generated.h"


UCLASS()
class LAWNMOWER_API AGrassSpawnerA : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) TSubclassOf<class AGrassA> GrassClass{};

public:
	AGrassSpawnerA();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	int32 GrassSpawned{};

};
