// Derived AActor class AGrassSpawnerV1 by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawnerV1.generated.h"


UCLASS()
class LAWNMOWER_API AGrassSpawnerV1 : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Mesh {};
	UPROPERTY(EditDefaultsOnly) TSubclassOf<class AGrassV1> GrassClass{};

public:
	AGrassSpawnerV1();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	int32 GrassSpawned{};

};
