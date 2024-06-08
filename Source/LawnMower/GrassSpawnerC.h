// GrassSpawnerC actor class developed by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawnerC.generated.h"


UCLASS()
class LAWNMOWER_API AGrassSpawnerC : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Mesh {};
	UPROPERTY(EditDefaultsOnly) TSubclassOf<class AGrass> GrassClass{};

public:	
	AGrassSpawnerC();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	int32 GrassSpawned{};

};
