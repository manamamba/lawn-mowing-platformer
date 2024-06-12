// Derived AActor class AGrassPoolA by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassPoolA.generated.h"

class AGrassD;


UCLASS()
class LAWNMOWER_API AGrassPoolA : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) USceneComponent* Root {};
	
public:	
	AGrassPoolA();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	TArray<AGrassD*> PooledGrass{};
};
