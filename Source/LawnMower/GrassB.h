// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassB.generated.h"


UCLASS()
class LAWNMOWER_API AGrassB : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = TSubClass) TSubclassOf<AGrassB> GrassBClass{};

public:	
	AGrassB();

private:
	void CreateAndAssignRootComponent();
	void AssignStaticMesh();

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
	bool FarGroundHitBySpawnerRayCast(FHitResult& Hit, const FVector& Start, const FVector& Direction, const double& PitchMax);
	bool GrassHitBySpawnerSweep(const FVector& Start, const FVector& Direction, const double& RayCastLength, const double& PitchMax);
	bool GroundHitBySpawnerRayCast(FHitResult& Hit, const FVector& Start, const FVector& Direction, const double& RayCastLength, const double& PitchMax);
	void SpawnGrass(FHitResult& Hit, const double& PitchMax);

	void UpdateRotatorYawAndPitch(const double& PitchMax);
	void UpdateRotatorPitch(const double& PitchMax);

	void DestroyRuntimeSpawningComponentsAndDisableTick();

	void TickSlowerWithDrawing();
	void DrawSpawningComponents();

private:
	UStaticMesh* StaticMesh{};
	UStaticMeshComponent* Mesh{};
	USceneComponent* Rotator{};
	USceneComponent* Spawner{};

	FRotator RotatorRotation{};
	FTransform RootTransform{};

	double StartingRotatorYaw{};

	float TickCount{};
	int32 SpawningCompleteTicks{ 1 };

};
