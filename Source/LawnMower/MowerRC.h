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

	UPROPERTY(EditDefaultsOnly, Category = Input) UInputMappingContext* InputMappingContext {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* MoveCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* ResetCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* BrakeInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* SteerInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* DriftInputAction {};

	const float GravitationalAcceleration{ 980.0f };
	const float PhysicsBodyMass{ 30.0f };
	const float PhysicsBodyAntiGravitationalForce{ PhysicsBodyMass * GravitationalAcceleration };

	const double PhysicsBodyCenterOfMassOffset{ PhysicsBodyMass / 2.0 };

	const FVector PhysicsBodyDimensions{ 30.5, 20.0, 9.0 };
	const FVector PhysicsBodyCenterOfMass{ 0.0, 0.0, -PhysicsBodyCenterOfMassOffset };

	const FRotator DefaultLocalCameraArmRotation{ -25.0, 0.0, 0.0 };

	const double MinLocalCameraArmPitch{ -89.9 };
	const double MaxLocalCameraArmPitch{ 89.9 };

	const FVector BodyPosition{ -0.3, 0.0, -1.0 };
	const FVector HandlePosition{ -23.3, 0.0, 0.0 };

	const FVector FrWheelPosition{ 26.0, 24.0, -9.0 };
	const FVector FlWheelPosition{ 26.0, -24.0, -9.0 };
	const FVector BrWheelPosition{ -26.0, 24.0, -9.0 };
	const FVector BlWheelPosition{ -26.0, -24.0, -9.0 };

	const FLocalOrigins WheelRayCastOrigins{ FrWheelPosition, FlWheelPosition, BrWheelPosition, BlWheelPosition };

	const FVector FrRayCastPosition{ 25.0, 15.0, -9.0 };
	const FVector FlRayCastPosition{ 25.0, -15.0, -9.0 };
	const FVector BrRayCastPosition{ -25.0, 15.0, -9.0 };
	const FVector BlRayCastPosition{ -25.0, -15.0, -9.0 };

	const FLocalOrigins ForceRayCastOrigins{ FrRayCastPosition, FlRayCastPosition, BrRayCastPosition, BlRayCastPosition };
	
	const double RayCastLength{ 8.9 };

	const float TickCountMultiplier{ 8.0f };

	const float AccelerationRatioMaximum{ 3.0f };
	const float AccelerationForceMaximum{ 20000.0f };
	const float AccelerationDecayRate{ 0.5f };
	const float AccelerationBrakingRate{ 1.75f };
	const float AccelerationDecayBrakingRate{ 0.5f };
	const float AccelerationRatioBrakingMinimum{ 0.15f };

	const double SteeringTorque{ 300.0 };
	const double DriftingForcePositionOffset{ 25.0 };
	const double DriftingForceMaximum{ 5000.0 };

	const float DriftingRatioMaximum{ 3.0f };				// decay slower, but start at a higher acc ratio
	const float DriftingForceIncreaseRate{ 2.0f };
	const float DriftingForceDecayRate{ 1.5f };
	const float DriftingAccelerationMinimum{ 0.0f };
	
	const float CompressionRatioMinimum{ 0.25f };
	const float MaxWheelDrag{ 2.0f };
	const float WheelTotal{ 4.0f };
	const float BrakingLinearDragIncreaseRate{ 0.15f };
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

private:
	void MoveCamera(const FInputActionValue& Value);
	void ResetCamera(const FInputActionValue& Value);
	void Accelerate(const FInputActionValue& Value);
	void Brake(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);
	void Drift(const FInputActionValue& Value);

private:
	FVector2D RotatingCameraDirection{};

	bool bCameraReset{};

	float AcceleratingDirection{};
	float Braking{};
	float Steering{};
	float Drifting{};

public:
	virtual void Tick(float DeltaTime) override;

private:
	void Float() const;

	void UpdateTransforms();
	void UpdateSpeed();
	void UpdateCameraRotation();

	void SendForceRayCasts(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins);
	bool ForceRayCastHit(FHitResult& RayCast, const FVector& LocalOrigin);
	void AddForcesOnRayCastHit(FHitResult& RayCast);
	void AddDragOnRayCastHit(float CompressionRatio);

	void UpdateAcceleratingConditionals();
	void UpdateAccelerationRatio(float DeltaTime);
	void UpdateDriftingRatio(float DeltaTime);
	void DecayRatio(float& Ratio, const float DecayRate, float DeltaTime);
	void LimitRatio(float& Ratio, const float RatioMaximum);
	void UpdateAcceleratingDirection();
	void ApplyAccelerationForce();
	void ApplySteeringTorque();
	void ApplyDriftingForce();

	void AddBrakingLinearDrag();
	void AddAcceleratingAngularDrag();
	void AddAirTimeAngularDrag();
	void ApplyDrag();
	
	void SendWheelSuspensionRayCasts(FRayCastGroup& RayCastGroup, const FLocalOrigins& LocalOrigins);
	void SetWheelSuspension(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin);

	void LogData(float DeltaTime);
	void UpdateTickCount(float DeltaTime);

	void ResetDrag();
	void ResetPlayerInputData();

	void DrawRayCastGroup(const FRayCastGroup& RayCasts) const;
	void DrawRayCast(const FHitResult& RayCast) const;
	void DrawAcceleration() const;

private:
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

	FHitResult FrForceRayCast{};
	FHitResult FlForceRayCast{};
	FHitResult BrForceRayCast{};
	FHitResult BlForceRayCast{};

	FRayCastGroup ForceRayCasts{ FrForceRayCast, FlForceRayCast, BrForceRayCast, BlForceRayCast };

	int32 WheelsGrounded{};

	float AccelerationForce{};
	float AccelerationRatio{};
	float DriftingRatio{};

	FVector AccelerationSurfaceImpact{};
	FVector AccelerationSurfaceNormal{};
	FVector DriftingForcePosition{};

	double SteeringForce{};
	double DriftingForce{};

	bool bMoving{};
	bool bAccelerating{};
	bool bSteering{};

	bool bDriftingAccelerationMinimum{}; // remove?

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

	float TickCount{};

	bool bTickReset{};

};