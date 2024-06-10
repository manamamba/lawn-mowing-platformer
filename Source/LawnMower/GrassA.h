// Derived AActor class AGrassA by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassA.generated.h"


UCLASS()
class LAWNMOWER_API AGrassA : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Mesh {};
	UPROPERTY(EditDefaultsOnly, Category = TSubClass) TSubclassOf<AGrassA> GrassClass{};

public:
	AGrassA();

protected:
	virtual void BeginPlay() override;

private:
	void RandomizeRotationAndScale();
	void CreateAndAttachSpawningComponents();
	void SetSpawningComponentsRotationAndLocation();

	void LogComponentsAttachedAtRuntime();

public:
	virtual void Tick(float DeltaTime) override;

private:
	void TickSlowerWithDrawing();

	void TryToSpawnGrass();
	bool GroundHitBySpawnerRayCast(FHitResult& Hit);
	bool GrassHitBySpawnerSweep(FHitResult& Hit) const;
	void SpawnGrass(FHitResult& Hit);

	void UpdateRotatorRotation();
	void ResetSpawningHammer();
	void DestroySpawningComponentsAndDisableTick();

	void DrawSpawningComponents();

private:
	USceneComponent* Rotator{};
	USceneComponent* Spawner{};

	FRotator RotatorRotation{};
	FTransform RootTransform{};

	float TickCount{};
	int32 SpawningCompleteTicks{ 1 };
};
