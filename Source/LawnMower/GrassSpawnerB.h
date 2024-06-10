// Derived AActor class AGrassSpawnerB by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawnerB.generated.h"

class UBoxComponent;
class AGrassC;


UCLASS()
class LAWNMOWER_API AGrassSpawnerB : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) USceneComponent* Root {};
	UPROPERTY(EditAnywhere) UBoxComponent* Collider {};
	UPROPERTY(EditAnywhere) TSubclassOf<AGrassC> GrassClass{};

public:
	AGrassSpawnerB();

private:
	void SetComponentProperties();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void TryToSpawnGrass();
	bool SpawnerHitGround(FHitResult& Hit) const;
	void SpawnGrass(const FHitResult& Hit);

	bool SpawnedGrassCleared() const;
	void UpdateGameModeGrassCut() const;
	void ActivateActorByTag();
	void DisableSpawnerTick();

public:
	int32 GetGrassType() const;
	int32 GetGrassSpawned() const;
	int32 GetGrassCut() const;

	void UpdateGrassSpawnedCount();
	void UpdateGrassCutCount();

private:
	UFUNCTION() void ActivateSpawner(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) int32 GrassType {};
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) FName ActorTag {};
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) bool bTriggerActorByTag{};
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) bool bSetNewSpawner{};

	int32 GrassSpawnedCount{};
	int32 GrassCutCount{};

	bool bSpawnerActivated{};
	bool bSpawnSuccessful{};

};
