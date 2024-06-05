// Mower pawn class developed by Cody Wheeler.


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


struct FRayCastGroup {
	FHitResult Fr{};
	FHitResult Fl{};
	FHitResult Br{};
	FHitResult Bl{};
};

struct FLocalOrigins {
	FVector Fr{};
	FVector Fl{};
	FVector Br{};
	FVector Bl{};
};


UCLASS()
class LAWNMOWER_API AMowerRC : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Component) UBoxComponent* PhysicsBody {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Body {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Handle {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* FrWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* FlWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* BrWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* BlWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) USpringArmComponent* CameraArm {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UCameraComponent* Camera {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UBoxComponent* Collider {};

	UPROPERTY(EditDefaultsOnly, Category = Input) UInputMappingContext* InputMappingContext {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* MoveCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* ResetCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* PitchInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* BrakeInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* SteerInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* DriftInputAction {};

	const float GravitationalAcceleration{ 980.0f };
	const float PhysicsBodyMass{ 30.0f };
	const float PhysicsBodyAntiGravitationalForce{ PhysicsBodyMass * GravitationalAcceleration };
	const double PhysicsBodyCenterOfMassOffset{ PhysicsBodyMass / 2.0 };
	const FVector PhysicsBodyCenterOfMass{ 0.0, 0.0, -PhysicsBodyCenterOfMassOffset };
	const FVector PhysicsBodyDimensions{ 30.5, 20.0, 9.0 };
	const FVector ColliderDimensions{ 30.5, 26.0, 4.0 };

	const FVector ColliderPosition{ 0.0, 0.0, -13.0 };
	const FVector BodyPosition{ -0.3, 0.0, -1.0 };
	const FVector HandlePosition{ -23.3, 0.0, 0.0 };
	const FVector FrWheelPosition{ 26.0, 24.0, -9.0 };
	const FVector FlWheelPosition{ 26.0, -24.0, -9.0 };
	const FVector BrWheelPosition{ -26.0, 24.0, -9.0 };
	const FVector BlWheelPosition{ -26.0, -24.0, -9.0 };
	const FLocalOrigins WheelRayCastOrigins{ FrWheelPosition, FlWheelPosition, BrWheelPosition, BlWheelPosition };

	const FRotator DefaultLocalCameraArmRotation{ -20.0, 0.0, 0.0 };
	const double MaxLocalCameraArmPitch{ 89.9 };
	const double RotationMaximum{ 360.0 };

	const FVector FrRayCastPosition{ 25.0, 15.0, -9.0 };
	const FVector FlRayCastPosition{ 25.0, -15.0, -9.0 };
	const FVector BrRayCastPosition{ -25.0, 15.0, -9.0 };
	const FVector BlRayCastPosition{ -25.0, -15.0, -9.0 };
	const FLocalOrigins ForceRayCastOrigins{ FrRayCastPosition, FlRayCastPosition, BrRayCastPosition, BlRayCastPosition };
	const double RayCastLength{ 8.9 };
	const float CompressionRatioMinimum{ 0.25f };
	const float MaxWheelDrag{ 2.0f };
	const float WheelTotal{ 4.0f };

	const float AccelerationForceMaximum{ 20000.0f };
	const float AccelerationRatioMaximum{ 3.0f };
	const float AccelerationDecayRate{ 0.5f };
	const float AccelerationRatioMinimumWhileBraking{ 0.15f };
	const float AccelerationBrakingRate{ 1.75f };
	const float AccelerationBrakingDecayRate{ 0.5f };

	const double SteeringForceMaximum{ 300.0 };
	const double SteeringForceOnSlopeRate{ 0.1 };

	const double DriftingForceMaximum{ 5000.0 };
	const double DriftingForcePositionOffset{ 25.0 };
	const float DriftingRatioMaximum{ 3.0f };
	const float DriftingRate{ 3.0f };
	const float DriftingDecayRate{ 3.0f };

	const double AirTimeAntiGravitationalForce{ PhysicsBodyMass * 245.0 };
	const float AirTimeRatioMaxium{ 3.0 };
	const float AirTimeRatioIncreaseRate{ 2.0 };
	const float AirTimeMinimum{ 0.25f };
	const float AirTimePitchForce{ 30000000.0 };
	const float AirTimeRollForce{ 15000000.0 };
	
	const float BrakingLinearDragRate{ 0.25f };
	const float BrakingLinearDragLimit{ 50.0f };
	const float AcceleratingAngularDragRate{ 0.00002f };
	const float AirTimeAngularDrag{ 5.0f };

	const double WheelAcceleratingPitchRate{ -1200.0 };
	const double WheelDriftingPitchRate{ -300.0 };
	const double WheelOnSlopePitchRate{ 2.0 };
	const double WheelSteeringMaximum{ 8.0 };
	const float WheelPitchMinimumWhileDrifting{ 0.001f };
	const float WheelSteeringRatioMaximum{ 3.0f };
	const float WheelSteeringRate{ 8.0f };
	const float WheelSteeringDecayRate{ 12.0f };

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
	void SetPhysicsBodyProperties();
	void SetCameraArmRotation();
	void AddInputMappingContextToLocalPlayerSubsystem() const;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void MoveCamera(const FInputActionValue& Value);
	void ResetCamera(const FInputActionValue& Value);
	void Accelerate(const FInputActionValue& Value);
	void Pitch(const FInputActionValue& Value);
	void Brake(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);
	void Drift(const FInputActionValue& Value);

private:
	FVector2D RotatingCameraDirection{};
	bool bCameraReset{};
	float AcceleratingDirection{};
	float PitchingDirection{};
	float Braking{};
	float Steering{};
	float Drifting{};

public:
	virtual void Tick(float DeltaTime) override;

private:
	void GetTickTime();

	void Float() const;

	void UpdateTransforms();

	void UpdateSpeed();

	void UpdateCameraRotation();
	void ResetFullAxisRotations(FRotator& Rotator) const;

	void UpdateHoveringForces(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins);
	bool ForceRayCastHitGround(FHitResult& RayCast, const FVector& LocalOrigin);
	void ApplyHoveringForce(FHitResult& RayCast);
	void AddHoveringForceDrag(const float CompressionRatio);

	void UpdateGroundedMovementConditions();
	void UpdateAccelerationRatio(const float DeltaTime);
	void UpdateDriftingRatio(const float DeltaTime);
	void DecayRatio(float& Ratio, const float DecayRate, const float DeltaTime);
	void LimitRatio(float& Ratio, const float RatioMaximum);
	void UpdateAcceleratingDirection();
	void ApplyAccelerationForce();
	void ApplySteeringTorque();
	void ApplyDriftingForce();

	void UpdateAirTimeRatio(const float DeltaTime);
	void ApplyAirTimeAntiGravitationalForce();
	void ApplyAirTimePitch();
	void ApplyAirTimeRoll();

	void AddBrakingLinearDrag();
	void AddAcceleratingAngularDrag();
	void AddAirTimeAngularDrag();
	void ApplyDrag();

	void UpdateWheelSuspension(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins);
	void SendWheelRayCast(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin);
	void ApplyWheelSuspension(UStaticMeshComponent* Wheel, const FHitResult& RayCast, const FVector& WheelStart, const bool Grounded);
	void UpdateWheelRotations(const float DeltaTime);
	double GetWheelPitch(const double PitchRate, const float Ratio, const float RatioMaximum, const float DeltaTime) const;
	float GetDriftingRatioWithPitchDirection() const;
	void UpdateWheelPitch(FRotator& LocalRotation, const double WheelPitch);	
	void UpdateWheelSteeringRatio(const float DeltaTime);
	void UpdateWheelYaw(FRotator& LocalRotation) const;
	void ApplyWheelRotation(UStaticMeshComponent* Wheel, const FRotator& LocalRotation) const;
	
	void DrawRayCastGroup(const FRayCastGroup& RayCasts) const;
	void DrawRayCast(const FHitResult& RayCast) const;
	void DrawAcceleration() const;
	void DrawDrift() const;

	void LogMotionData(const float DeltaTime);
	void UpdateTickCount(const float DeltaTime);

	void ResetDrag();
	void ResetPlayerInputData();

	void LogTickTime();

private:
	double TickTime{};
	double LongestTickTime{};

	FTransform PhysicsBodyWorldTransform{};
	FTransform PhysicsBodyLocalTransform{};
	FVector PhysicsBodyLocation{};
	FVector PhysicsBodyUpVector{};
	FVector PhysicsBodyForwardVector{};
	FVector PhysicsBodyRightVector{};

	FVector LocationThisTick{};
	FVector LocationLastTick{};
	double PhysicsBodySpeed{};

	mutable FRotator LocalCameraArmRotation{ DefaultLocalCameraArmRotation };
	mutable FRotator WorldCameraArmRotation{};

	FHitResult FrForceRayCast{};
	FHitResult FlForceRayCast{};
	FHitResult BrForceRayCast{};
	FHitResult BlForceRayCast{};
	FRayCastGroup ForceRayCasts{ FrForceRayCast, FlForceRayCast, BrForceRayCast, BlForceRayCast };
	int32 WheelsGrounded{};

	bool bMovingByAccumulatedAcceleration{};
	bool bAccelerating{};
	bool bSteering{};
	bool bLastAccelerationWasForward{};

	FVector AccelerationSurfaceImpact{};
	FVector AccelerationSurfaceNormal{};

	FVector DriftingForcePosition{};
	float AccelerationForce{};
	float AccelerationRatio{};
	float DriftingRatio{};

	float AirTimeRatio{};
	bool bAirTimeMinimumExceeded{};

	TArray<float> LinearDragArray{};
	TArray<float> AngularDragArray{};
	float TotalLinearDrag{};
	float TotalAngularDrag{};
	float LinearBrakingDrag{};

	FHitResult FrWheelRayCast{};
	FHitResult FlWheelRayCast{};
	FHitResult BrWheelRayCast{};
	FHitResult BlWheelRayCast{};
	FRayCastGroup WheelRayCasts{ FrWheelRayCast, FlWheelRayCast, BrWheelRayCast, BlWheelRayCast };
	mutable FRotator LocalFrontWheelRotations{};
	mutable FRotator LocalRearWheelRotations{};
	float WheelSteeringRatio{};

	float TickCount{};
	bool bTickReset{};
};