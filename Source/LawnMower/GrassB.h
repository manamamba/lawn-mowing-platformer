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
	void SetRuntimeComponentProperties();

public:
	virtual void Tick(float DeltaTime) override;

private:
	void TickSlowerWithDrawing();

	void TryToSpawnGrass();
	bool GroundHitBySpawnerRayCast(FHitResult& Hit);
	bool GrassHitBySpawnerSweep(FHitResult& Hit);
	void SpawnGrass(FHitResult& Hit);

	void UpdateRotatorRotation();
	void ResetSpawningHammer();
	bool DestroyRuntimeComponentsAndDisableTick();

	void DrawSpawningComponents();

private:
	UStaticMesh* StaticMesh{};
	UStaticMeshComponent* Mesh{};
	USceneComponent* Rotator{};
	USceneComponent* Spawner{};

	FRotator RotatorRotation{};
	FTransform RootTransform{};

	float TickCount{};
	int32 SpawningCompleteTicks{ 1 };


};
