// Derived AActor class AGrassD by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassD.generated.h"


UCLASS()
class LAWNMOWER_API AGrassD : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = TSubClass) TSubclassOf<AGrassD> GrassClass{};
	
	static inline const double RayCastLength{ 24.0 };

	static inline const TArray<FVector> LocalStarts{
		{ 10.0, 0.0, 12.0 },
		{ 5.0, 8.66, 12.0 },
		{ -5.0, 8.66, 12.0 },
		{ -10.0, 0.0, 12.0 },
		{ -5.0, -8.66, 12.0 },
		{ 5.0, -8.66, 12.0 }
	};

public:	
	AGrassD();

private:
	void CreateAndAssignRootComponent();
	void LocateAndAssignStaticMesh();

protected:
	virtual void BeginPlay() override;

private:
	void CreateAndAttachMeshComponent();
	void SetYawPositions();
	void SetComponentProperties();
	void SetRayCastStarts();

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UStaticMesh* StaticMesh{};
	UStaticMeshComponent* Mesh{};
	TArray<double> YawPositions{};
	TArray<FVector> RayCastStarts{};
	FTransform RootTransform{};

};
