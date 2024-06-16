// Derived AActor class APlanetoidA by Cody Wheeler.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetoidA.generated.h"

class AMowerB;
class UBoxComponent;
class USphereComponent;


UCLASS()
class LAWNMOWER_API APlanetoidA : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Mesh {};
	UPROPERTY(EditDefaultsOnly, Category = Component) USphereComponent* Collider {};

	const float GravitationalRadius{ 200.0f };

	UFUNCTION() void Capture(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION() void Release(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	const float GravitationalAcceleration{ 980.0f };

public:	
	APlanetoidA();

private:
	void SetComponentProperties();

protected:
	virtual void BeginPlay() override;

private:
	void SetMowerProperties();

public:	
	virtual void Tick(float DeltaTime) override;

private:
	void ApplyCaptureForce(const float DeltaTime);

private:
	UPROPERTY(EditAnywhere, Category = Settings, meta = (AllowPrivateAccess = "true")) float AntiGravityMultiplier{ 1.0f };

	AMowerB* Mower{};
	class UBoxComponent* MowerPhysicsBody{};
	double MowerMass{};
	double MowerCaptureForce{};
	float PlanetoidAcceleration{};
	bool MowerCaptured{};

};
