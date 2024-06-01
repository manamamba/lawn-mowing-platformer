// Grass actor class developed by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grass.generated.h"

class UBoxComponent;


UCLASS()
class LAWNMOWER_API AGrass : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Mesh{};
	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Rotator {};
	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Spawner {};
	UPROPERTY(EditDefaultsOnly, Category = TSubClass) TSubclassOf<class AGrass> GrassClass{};
	
	const double RotatorYawRate{ 60.0 };
	const double RotatorPitchRate{ -15.0 };

public:	
	AGrass();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	bool bRotatorYawRotationCompleted{};
	bool bRotatorPitchRotationCompleted{};

};
