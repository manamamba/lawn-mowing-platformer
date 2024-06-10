// Derived APawn class AMowerA by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MowerA.generated.h"

class UPhysicsConstraintComponent;
class UArrowComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;


struct FWheelSet {
	UStaticMeshComponent* Wheel{};
	UPhysicsConstraintComponent* Axis{};
	UPhysicsConstraintComponent* Suspension{};
	const FVector Location{};
	FName WheelName{};
};


UCLASS()
class LAWNMOWER_API AMowerA : public APawn
{
	GENERATED_BODY()

public:
	AMowerA();
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
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Body {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Handle {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* WheelFr {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* WheelFl {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* WheelBr {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* WheelBl {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* AxisFr {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* AxisFl {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* AxisBr {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* AxisBl {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* SuspensionFr {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* SuspensionFl {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* SuspensionBr {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UPhysicsConstraintComponent* SuspensionBl {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UArrowComponent* Arrow {};
	UPROPERTY(EditDefaultsOnly, Category = Component) USpringArmComponent* CameraArm {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UCameraComponent* Camera {};

	UPROPERTY(EditDefaultsOnly, Category = Input) UInputMappingContext* MowerInputMappingContext {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* MoveCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* ResetCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* BrakeInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* SteerInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* JumpInputAction {};

	const FVector LocationFr{ 24.0, 24.0, -8.0 };
	const FVector LocationFl{ 24.0, -24.0, -8.0 };
	const FVector LocationBr{ -24.0, 24.0, -8.0 };
	const FVector LocationBl{ -24.0, -24.0, -8.0 };
	const double MinArmPitch{ -89.0 };
	const double MaxArmPitch{ 8.0 };
	const double AccelerationPower{ 300.0 };
	const float MaxWheelDrag{ 10.25f };
	const double SteeringPower{ 10.0 };

	FRotator SteeringPosition{};
	float WheelDrag{};
	bool bBraking{};

private:
	void CreateAndAssignComponentSubObjects();
	void SetupComponentAttachments();
	void SetNonWheelProperties();
	void SetWheelProperties(FWheelSet Set);
	void SetMeshCollisionProperties(UStaticMeshComponent* Mesh);
	void SetMeshMassOverrides();
	void AddInputMappingContextToLocalPlayerSubsystem() const;
	void ReduceWheelDrag();
	void SetRearWheelAngularDamping();
	bool IsGrounded(APawn* Mower) const;

};
