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

public:	
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateLocation(const FVector& Origin, const FVector& Target, const float DeltaTime);

private:
	UPROPERTY(EditAnywhere, Category = Settings, meta = (AllowPrivateAccess = "true")) FVector LocationOffset {};
	UPROPERTY(EditAnywhere, Category = Settings, meta = (AllowPrivateAccess = "true")) double MovingSpeed {};
	UPROPERTY(EditAnywhere, Category = Settings, meta = (AllowPrivateAccess = "true")) float MaxWaitTimeInSeconds {};
	UPROPERTY(EditAnywhere, Category = Settings, meta = (AllowPrivateAccess = "true")) bool bMoveOnce {};

	FTransform RootTransform{};
	FVector StartLocation{};
	FVector EndLocation{};
	FVector MovingDirection{};
	double MaxDistance{};
	bool bMovingToEndLocation{ true };
	float WaitTimeInSeconds{};

};
