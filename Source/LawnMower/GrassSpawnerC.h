// Derived AActor class AGrassSpawnerB by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawnerC.generated.h"

class UBoxComponent;


UCLASS()
class LAWNMOWER_API AGrassSpawnerC : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) USceneComponent* Root {};
	UPROPERTY(EditAnywhere) UBoxComponent* Collider {};

public:	
	AGrassSpawnerC();

private:
	void SetComponentProperties();

protected:
	virtual void BeginPlay() override;

private:
	void SetGrassPoolOwner();

public:	
	virtual void Tick(float DeltaTime) override;

private:
	void TryToSpawnGrass();
	bool RayCastHitGround(FHitResult& Hit) const;
	void SpawnGrass(const FHitResult& Hit);

	bool SpawnedGrassCleared() const;
	void UpdateGameMode() const;
	void ActivateActorByTag();

public:
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
	// UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) FName ActorTag {};
	// UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) bool bTriggerActorByTag{};
	// UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true")) bool bSetNewSpawner{};

	//race condition issue, need to use an array here instead of int32

	int32 GrassSpawnedCount{};
	int32 GrassCutCount{};

	bool bSpawnSuccessful{};

};
