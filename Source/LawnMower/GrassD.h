// Derived AActor class AGrassD by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassD.generated.h"

class AGrassPoolA;
class AGrassSpawnerC;


UCLASS()
class LAWNMOWER_API AGrassD : public AActor
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

	static inline const float TickCountMax{ 0.045f }; //was 0.015f

public:	
	AGrassD();

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
	void TryToSpawnGrass();
	bool RayCastStartInsideGrowField(const FVector& RayCastStart);
	bool RayCastHitGround(FHitResult& Hit, const FVector& RayCastStart, const FVector& RayCastDirection);
	bool GrassNearRayCastImpact(const FVector& Impact);
	void SpawnGrass(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	FRotator GetSpawnRotation(const FTransform& Transform, const float& TraceLength, const double& YawPosition);

private:
	AGrassSpawnerC* SpawnOwner{};
	AGrassPoolA* PoolOwner{};
	UStaticMeshComponent* Mesh{};
	FTransform RootTransform{};
	FVector RootDownVector{};
	float TickCount{};
	int32 SpawnAttempts{};
	bool bCut{};

};
