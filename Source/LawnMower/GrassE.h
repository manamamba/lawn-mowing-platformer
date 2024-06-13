// Derived AActor class AGrassE by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassE.generated.h"


class AGrassSpawnerD;


UCLASS()
class LAWNMOWER_API AGrassE : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};

	static inline UStaticMesh* StaticMesh{};

	static inline const TArray<FVector> LocalRayCastStarts{
		{ 14.0, 0.0, 12.0 },
		{ 7.0, 12.124, 12.0 },
		{ -7.0, 12.124, 12.0 },
		{ -14.0, 0.0, 12.0 },
		{ -7.0, -12.124, 12.0 },
		{ 7.0, -12.124, 12.0 }
	};

	static inline const TArray<double> YawRotations{ 0.0, 60.0, 120.0, 180.0, 240.0, 300.0 };

	static inline const double RayCastLength{ 24.0 };

public:
	AGrassE();

private:
	void CreateAndAssignRootComponent();
	void LocateAndAssignStaticMesh();

protected:
	virtual void BeginPlay() override;

private:
	void CreateAndAttachMeshComponent();
	void SetMeshComponentProperties();

	UFUNCTION() void Cut(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:
	virtual void Tick(float DeltaTime) override;

private:
	bool ReadyToTrySpawning(const float& DeltaTime);
	void TryToSpawnGrass();
	bool RayCastStartInsideGrowField(const FVector& RayCastStart);
	bool RayCastHitGround(FHitResult& Hit, const FVector& RayCastStart, const FVector& RayCastDirection);
	bool GrassNearRayCastImpact(const FVector& Impact);
	void SpawnGrass(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	FRotator GetSpawnRotation(const FTransform& Transform, const float& TraceLength, const double& YawPosition);

private:
	UStaticMeshComponent* Mesh{};
	AGrassSpawnerD* SpawnOwner{};
	FTransform RootTransform{};
	FVector RootDownVector{};
	int32 SpawnAttempts{};
	float TickCount{};
	float TickCountThreshold{ 0.016f };

};
