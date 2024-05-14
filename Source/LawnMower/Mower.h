// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Mower.generated.h"

class UPhysicsConstraintComponent;
class UArrowComponent; 
class USpringArmComponent;
class UCameraComponent;

struct WheelSet {
	UStaticMeshComponent* Wheel{};
	UPhysicsConstraintComponent* Axis{};
	UPhysicsConstraintComponent* Suspension{};
	const FVector Location{};
	FName Name{};
	FName RootName{ TEXT("Body") };
};


UCLASS()
class LAWNMOWER_API AMower : public APawn
{
	GENERATED_BODY()

public:
	AMower();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

public:	
	

private:
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Body{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Handle{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* WheelFR{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* WheelFL{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* WheelBR{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* WheelBL{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* AxisFR{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* AxisFL{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* AxisBR{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* AxisBL{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* SuspensionFR{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* SuspensionFL{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* SuspensionBR{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* SuspensionBL{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UArrowComponent* Arrow{};
	UPROPERTY(EditDefaultsOnly, Category = Component) USpringArmComponent* CameraArm{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UCameraComponent* Camera{};

	const FVector FRPlacement{ 24.0, 24.0, -8.0 };
	const FVector FLPlacement{ 24.0, -24.0, -8.0 };
	const FVector BRPlacement{ -24.0, 24.0, -8.0 };
	const FVector BLPlacement{ -24.0, -24.0, -8.0 };

private:
	void SetWheelProperties(WheelSet WheelSet);


};
