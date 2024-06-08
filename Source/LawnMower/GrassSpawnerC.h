// GrassSpawnerC actor class developed by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawnerC.generated.h"

class UBoxComponent;
class AGrassC;


UCLASS()
class LAWNMOWER_API AGrassSpawnerC : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) USceneComponent* Root {};
	UPROPERTY(EditAnywhere) UBoxComponent* Collider {};
	UPROPERTY(EditAnywhere) TSubclassOf<AGrassC> GrassClassC{};

	UFUNCTION() void ActivateSpawner(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:
	AGrassSpawnerC();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void TryToSpawn();
	void DisableSpawnerTick();

public:
	int32 GetGrassType() const;
	void UpdateGrassSpawnedCount();

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) int32 GrassType {};

	int32 GrassSpawnedCount{};

	bool bSpawnerActivated{};
	bool bSpawnSuccessful{};

};
