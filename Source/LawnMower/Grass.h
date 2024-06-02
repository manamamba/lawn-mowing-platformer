// Grass actor class developed by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grass.generated.h"


UCLASS()
class LAWNMOWER_API AGrass : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Mesh {};
	UPROPERTY(EditDefaultsOnly, Category = TSubClass) TSubclassOf<AGrass> GrassClass{};

public:
	AGrass();

protected:
	virtual void BeginPlay() override;

private:
	void RandomizeRotationAndScale();
	void CreateAndAttachSpawningComponents();
	void SetSpawningComponentPositions();

	void LogComponentsAttachedAtRuntime();

public:
	virtual void Tick(float DeltaTime) override;

private:
	void TickSlower();

	void TryToSpawnGrass();
	bool GroundHitBySpawnerRayCast(FHitResult& Hit);
	bool GrassHitBySpawnerSweep(FHitResult& Hit);
	void SpawnGrass(FHitResult& Hit);

	void UpdateRotatorRotation();
	void DestroySpawningComponentsAndDisableTick();

	void DrawSpawningComponents();

private:
	USceneComponent* Rotator{};
	USceneComponent* Spawner{};

	FRotator RotatorRotation{};

	float TickCount{};
};
