// Derived AActor class AGrassSpawnerB by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawnerD.generated.h"

class AGrassE;
class UBoxComponent;


UCLASS()
class LAWNMOWER_API AGrassSpawnerD : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) USceneComponent* Root {};
	UPROPERTY(EditAnywhere) UBoxComponent* Collider {};

public:
	TArray<AGrassE*> GrassChildrenActivelySpawning{};

public:
	AGrassSpawnerD();

private:
	void SetComponentProperties();

	UFUNCTION() void ActivateSpawner(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void TryToSpawnGrass();
	bool RayCastHitGround(FHitResult& Hit) const;
	void SpawnGrass(const FHitResult& Hit);

	bool SpawnedGrassCleared() const;
	void UpdateGameMode() const;
	void UpdateMowerRespawn();
	void ActivateActorByTag();

public:
	void UpdateGrassSpawnedCount();
	void UpdateGrassCutCount();

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) FName ActorTag {};
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) bool bActivateActorByTag{};
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) bool bSetNewMowerSpawn{};

	int32 GrassSpawnedCount{};
	int32 GrassCutCount{};

};
