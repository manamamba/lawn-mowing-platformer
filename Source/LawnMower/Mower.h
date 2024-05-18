// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Mower.generated.h"

class UPhysicsConstraintComponent;
class UArrowComponent; 
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;


struct WheelSet {
	UStaticMeshComponent* Wheel{};
	UPhysicsConstraintComponent* Axis{};
	UPhysicsConstraintComponent* Suspension{};
	const FVector Location{};
	FName WheelName{};
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
	void MoveCamera(const FInputActionValue& Value);
	void ResetCamera();
	void Accelerate(const FInputActionValue& Value);
	void Brake(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);
	void Jump(const FInputActionValue& Value);

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

	UPROPERTY(EditDefaultsOnly, Category = Input) UInputMappingContext* MowerInputMappingContext{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* MoveCameraInputAction{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* ResetCameraInputAction{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* BrakeInputAction{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* SteerInputAction{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* JumpInputAction{};

	const FVector LocationFR{ 24.0, 24.0, -8.0 };
	const FVector LocationFL{ 24.0, -24.0, -8.0 };
	const FVector LocationBR{ -24.0, 24.0, -8.0 };
	const FVector LocationBL{ -24.0, -24.0, -8.0 };
	const double MinArmPitch{ -89.0 };
	const double MaxArmPitch{ 8.0 };
	const double AccelerationPower{ 300.0 };
	const float MaxWheelDrag{ 10.25f };
	const double SteeringPower{ 10.0 };

	FRotator SteeringPosition{};
	float WheelDrag{};
	bool Braking{};

private:
	void CreateAndAssignComponentSubObjects();
	void SetupComponentAttachments();
	void SetNonWheelProperties();
	void SetWheelProperties(WheelSet Set);
	void SetMeshCollisionProperties(UStaticMeshComponent* Mesh);
	void SetMeshMassOverrides();
	void AddInputMappingContextToLocalPlayerSubsystem();
	void ReduceWheelDrag();
	void SetRearWheelAngularDamping();
	bool IsGrounded(APawn* Mower);

};