// Player controlled lawn mower class by Cody Wheeler


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MowerRC.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;


struct RayCastGroup {
	FHitResult FR{};
	FHitResult FL{};
	FHitResult BR{};
	FHitResult BL{};
};

struct LocalOrigins {
	FVector FR{};
	FVector FL{};
	FVector BR{};
	FVector BL{};
};


UCLASS()
class LAWNMOWER_API AMowerRC : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) UBoxComponent* PhysicsBody {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Body {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Handle {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* FRWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* FLWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* BRWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* BLWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) USpringArmComponent* CameraArm {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UCameraComponent* Camera {};

	UPROPERTY(EditDefaultsOnly, Category = Input) UInputMappingContext* InputMappingContext {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* MoveCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* ResetCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* BrakeInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* SteerInputAction {};

	const FVector BodyPosition{ -0.3, 0.0, -1.0 };
	const FVector HandlePosition{ -23.3, 0.0, 0.0 };

	const FVector FRWheelPosition{ 26.0, 24.0, -9.0 };
	const FVector FLWheelPosition{ 26.0, -24.0, -9.0 };
	const FVector BRWheelPosition{ -26.0, 24.0, -9.0 };
	const FVector BLWheelPosition{ -26.0, -24.0, -9.0 };

	const FVector FRRayCastPosition{ 25.0, 15.0, -9.0 };
	const FVector FLRayCastPosition{ 25.0, -15.0, -9.0 };
	const FVector BRRayCastPosition{ -25.0, 15.0, -9.0 };
	const FVector BLRayCastPosition{ -25.0, -15.0, -9.0 };

	const LocalOrigins ForceRayCastOrigins{ FRRayCastPosition, FLRayCastPosition, BRRayCastPosition, BLRayCastPosition };
	const LocalOrigins WheelRayCastOrigins{ FRWheelPosition, FLWheelPosition, BRWheelPosition, BLWheelPosition };

	const double RayCastLength{ 8.9 };

	const float GravitationalAcceleration{ 980.0f };
	const float PhysicsBodyMass{ 30.0f };
	const float PhysicsBodyAntiGravitationalForce{ PhysicsBodyMass * GravitationalAcceleration };

	const double PhysicsBodyCenterOfMassOffset{ PhysicsBodyMass / 2.0 };

	const FVector PhysicsBodyDimensions{ 30.5, 20.0, 9.0 };
	const FVector PhysicsBodyCenterOfMass{ 0.0, 0.0, -PhysicsBodyCenterOfMassOffset };
	
	const float AccelerationRatioMaximum{ 3.0f };
	const float AccelerationForceMaximum{ 10000.0f };
	const float AcceleratingDecayRate{ 0.5f };

	const double BrakingForceOffset{ 25.0 };
	const double SteeringForce{ 850.0 };

	const FRotator DefaultLocalCameraArmRotation{ -25.0, 0.0, 0.0 };

	const double MinLocalCameraArmPitch{ -89.9 };
	const double MaxLocalCameraArmPitch{ 89.9 };

	const float CompressionRatioMinimum{ 0.25f };
	const float MaxWheelDrag{ 2.0f };
	const float WheelTotal{ 4.0f };
	const float BrakingLinearDragIncreaseRate{ 0.1f };
	const float BrakingLinearDragLimit{ 50.0f };
	const float AcceleratingAngularDragMultiplier{ 0.00002f };
	const float AirTimeAngularDrag{ 5.0f };

public:
	AMowerRC();

private:
	void CreateAndAssignComponentSubObjects();
	void SetupComponentAttachments();
	void SetComponentProperties();
	void SetMeshComponentCollisionAndLocation(UStaticMeshComponent* Mesh, const FVector& Location);

protected:
	virtual void BeginPlay() override;

private:
	void AddInputMappingContextToLocalPlayerSubsystem() const;
	void SetPhysicsBodyProperties();
	void SetCameraArmWorldRotation();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveCamera(const FInputActionValue& Value);
	void ResetCamera(const FInputActionValue& Value);
	void Accelerate(const FInputActionValue& Value);
	void Brake(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);

private:
	FVector2D RotatingCameraDirection{};
	bool CameraReset{};
	float AcceleratingDirection{};
	float Braking{};
	float Steering{};

public:
	virtual void Tick(float DeltaTime) override;

private:
	void TickCounter(float DeltaTime);

	void FloatMower() const;

	void UpdateAccelerationRatio(float DeltaTime);
	void UpdateAccelerationPositionData();
	void UpdateAccelerationForce();
	void ApplyAccelerationForce() const;
	void ApplySteeringTorque() const;
	void ApplyBrakingForce();

	void ResetDrag();

	void UpdatePhysicsBodyPositionData();
	void UpdatePhysicsBodySpeed(float DeltaTime);
	void UpdateCameraRotation();

	void SendForceRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins);
	bool RayCastHit(FHitResult& RayCast, const FVector& LocalOrigin);
	void AddForcesOnRayCastHit(FHitResult& RayCast);
	void AddDragOnRayCastHit(float CompressionRatio);

	void SendWheelRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins);
	void ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin);

	void AddBrakingLinearDrag();
	void AddAcceleratingAngularDrag();
	void AddAirTimeAngularDrag();
	void ApplyDrag();

	void ResetPlayerInputData();

	void LogData();
	void DrawRayCasts(RayCastGroup& RayCasts) const;
	void DrawRayCast(const FHitResult& RayCast) const;
	void DrawForces() const;
	

private:
	bool TickReset{};
	float TickCount{};

	float AccelerationForce{};
	float AccelerationRatio{};

	FVector AccelerationSurfaceImpact{};
	FVector AccelerationSurfaceNormal{};

	FTransform PhysicsBodyWorldTransform{};
	FTransform PhysicsBodyLocalTransform{};
	FVector PhysicsBodyLocation{};
	FVector PhysicsBodyUpVector{};
	FVector PhysicsBodyForwardVector{};
	FVector PhysicsBodyRightVector{};

	FVector LocationThisTick{};
	FVector LocationLastTick{};
	double PhysicsBodySpeed{};

	FRotator LocalCameraArmRotation{ DefaultLocalCameraArmRotation };
	FRotator WorldCameraArmRotation{};

	FHitResult FRForceRayCast{};
	FHitResult FLForceRayCast{};
	FHitResult BRForceRayCast{};
	FHitResult BLForceRayCast{};
	RayCastGroup ForceRayCasts{ FRForceRayCast, FLForceRayCast, BRForceRayCast, BLForceRayCast };

	FHitResult FRWheelRayCast{};
	FHitResult FLWheelRayCast{};
	FHitResult BRWheelRayCast{};
	FHitResult BLWheelRayCast{};
	RayCastGroup WheelRayCasts{ FRWheelRayCast, FLWheelRayCast, BRWheelRayCast, BLWheelRayCast };

	TArray<float> LinearDragArray{};
	TArray<float> AngularDragArray{};
	float TotalLinearDrag{};
	float TotalAngularDrag{};
	float LinearBrakingDrag{};
	float WheelsGrounded{};

};