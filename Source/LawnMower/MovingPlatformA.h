// Derived AActor class AMovingPlatformA by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatformA.generated.h"


UCLASS()
class LAWNMOWER_API AMovingPlatformA : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) USceneComponent* Root {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Mesh {};
	
public:	
	AMovingPlatformA();

protected:
	virtual void BeginPlay() override;

private:
	void SetComponentProperties();

private:
	void SetMovingData();
	void SetRotatingData();

public:	
	virtual void Tick(float DeltaTime) override;

private:
	void TryToMovePlatform(const float DeltaTime);
	void UpdateLocation(const FVector& Origin, const FVector& Target, const float DeltaTime);
	void TryToRotatePlatform(const float DeltaTime);
	void UpdateNonContinuousLocalRotation();
	void ResetFullAxisRotations(FRotator& Rotator) const;

private:
	UPROPERTY(EditAnywhere, Category = Activation, meta = (AllowPrivateAccess = "true")) bool bActiveWithoutSpawner{};

	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) bool bMoves {};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) bool bMovesOnce {};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) double MovingSpeed { 1.0 };
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) float WaitTimeInSecondsBeforeMoving {};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) FVector LocationOffset {};

	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) bool bRotates {};
	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) bool bRotatesOnce{};
	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) bool bContinuousRotation {};
	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) double RotationSpeed { 1.0 };
	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) float WaitTimeInSecondsBeforeRotating{};
	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) FRotator RotationOffset {};

	FVector StartLocation{};
	FVector EndLocation{};
	FVector MovingDirection{};
	double MovingDistance{};
	float TimeWaitedBeforeMoving{};
	bool bMovingToEndLocation{ true };

	mutable FRotator LocalRotation{};
	FRotator RotatingDirection{};
	float TimeWaitedBeforeRotating{};
	bool bPitchOffsetIsZero{};
	bool bYawOffsetIsZero{};
	bool bRollOffsetIsZero{};

};
