// Fill out your copyright notice in the Description page of Project Settings.


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
	void SetRotationData();

public:	
	virtual void Tick(float DeltaTime) override;

private:
	void TryToMovePlatform(const float DeltaTime);
	void UpdateLocation(const FVector& Origin, const FVector& Target, const float DeltaTime);
	void TryToRotatePlatform(const float DeltaTime);
	void ResetFullAxisRotations(FRotator& Rotator) const;

private:
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) bool bActiveWithoutSpawner{};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) bool bMoves {};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) bool bMovesOnce {};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) double MovingSpeed {};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) float WaitTimeInSecondsBeforeMoving {};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) FVector LocationOffset {};

	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) bool bRotates {};
	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) bool bContinuousRotation {};
	UPROPERTY(EditAnywhere, Category = Rotating, meta = (AllowPrivateAccess = "true")) double RotationSpeed {};
	UPROPERTY(EditAnywhere, Category = Moving, meta = (AllowPrivateAccess = "true")) FRotator RotationOffset {};

	FTransform RootTransform{};

	FVector StartLocation{};
	FVector EndLocation{};
	FVector MovingDirection{};
	double MovingDistance{};
	float TimeWaitedBeforeMoving{};
	bool bMovingToEndLocation{ true };

	FRotator LocalRotation{};
	FRotator RotatingDirections{};



};
