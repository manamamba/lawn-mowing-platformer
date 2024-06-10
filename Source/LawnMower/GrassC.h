// GrassC actor class developed by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassC.generated.h"


UCLASS()
class LAWNMOWER_API AGrassC : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = TSubClass) TSubclassOf<AGrassC> GrassClassC{};

public:
	enum EGrassType : int32 {
		Standard,
		Optional,

		Max_Grass_Types,
	};

public:
	AGrassC();

private:
	void CreateAndAssignRootComponent();
	void AssignStaticMesh();
	UStaticMesh* GetMeshType();

protected:
	virtual void BeginPlay() override;

private:
	void CreateAndAttachRuntimeComponents();
	void SetRuntimeMeshComponentProperties();
	void SetRuntimeSpawningComponentProperties();
	

public:
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateSpawnHammer();

	void TryToSpawnGrass(const double& PitchMax);
	bool FarDirtHitBySpawnerRayCast(FHitResult& Hit, const FVector& Start, const FVector& Direction, const double& PitchMax);
	bool GrowFieldOverlapped(const FVector& Start);
	bool GrassHitBySpawnerSweep(const FVector& Start, const FVector& Direction, const double& RayCastLength, const double& PitchMax);
	bool DirtHitBySpawnerRayCast(FHitResult& Hit, const FVector& Start, const FVector& Direction, const double& RayCastLength, const double& PitchMax);
	void SpawnGrass(FHitResult& Hit, const double& PitchMax);

	void UpdateRotatorYawAndPitch(const double& PitchMax);
	void UpdateRotatorPitch(const double& PitchMax);

	void DestroyRuntimeSpawningComponentsAndDisableTick();

	void TickSlowerWithDrawing();
	void DrawSpawningComponents();

private:
	UFUNCTION() void Cut(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	UStaticMesh* StaticMeshStandard{};
	UStaticMesh* StaticMeshOptional{};
	UStaticMeshComponent* Mesh{};
	USceneComponent* Rotator{};
	USceneComponent* Spawner{};

	FRotator RotatorRotation{};
	FTransform RootTransform{};

	double StartingRotatorYaw{};

	float TickCount{};
	int32 SpawningTicks{ 1 };

};
