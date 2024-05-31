// Grass spawner actor class developed by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawner.generated.h"


UCLASS()
class LAWNMOWER_API AGrassSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AGrassSpawner();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};