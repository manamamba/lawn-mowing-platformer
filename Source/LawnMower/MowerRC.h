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

	UPROPERTY(EditDefaultsOnly, Category = Component) UBoxComponent* PhysicsBody{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Body{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Handle{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* FRWheel{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* FLWheel{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* BRWheel{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* BLWheel{};
	UPROPERTY(EditDefaultsOnly, Category = Component) USpringArmComponent* CameraArm{};
	UPROPERTY(EditDefaultsOnly, Category = Component) UCameraComponent* Camera{};

	UPROPERTY(EditDefaultsOnly, Category = Input) UInputMappingContext* InputMappingContext{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* MoveCameraInputAction{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* ResetCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* BrakeInputAction{};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* SteerInputAction{};

	const float TickCountMax{ 1.0f };
	const float TickMultipler{ 8.0f };
	
	const FRotator CameraArmRotationOffset{ -30.0, 0.0, 0.0 };

	const FVector PhysicsBodyDimensions{ 30.5, 20.0, 9.0 };

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

	const float PhysicsBodyMass{ 30.0f };
	const float GravitationalAcceleration{ 980.0f };
	const float AntiGravitationalForce{ PhysicsBodyMass * GravitationalAcceleration };

	const FVector PhysicsBodyCenterOfMass{ 0.0, 0.0, -PhysicsBodyMass / 2.0 };

	const double MinCameraArmPitch{ -89.9 };
	const double MaxCameraArmPitch{ 89.9 };

	const double SurfaceImpactOffset{ -15.0 };

	const float AccelerationForceMaximum{ 10000.0f };
	const float AccelerationRatioMaximum{ 3.0f };
	const float AcceleratingDecayRate{ 0.5f };

	const double SteeringForce{ 6000000.0 };

	const double RayCastLength{ 8.9 };

	const float CompressionRatioMinimum{ 0.25f };
	const float MaxWheelDragForce{ 2.0f };
	const float WheelTotal{ 4.0f };

	const float LinearBrakingDragLimit{ 50.0f };
	const float LinearBrakingDragMultiplier{ 25.0f };
	const float AngularDragForceMultiplier{ 0.00002f };
	const float AngularAirTimeDrag{ 5.0f };

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
	void ResetCamera();
	void Accelerate(const FInputActionValue& Value);
	void Brake(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);

private:
	float AcceleratingDirection{};
	float Braking{};
	float Steering{};

public:
	virtual void Tick(float DeltaTime) override;

private:
	void TickCounter(float DeltaTime);

	void FloatMower() const;

	void UpdateCameraRotation();

	void UpdateAccelerationData(const RayCastGroup& RayCastGroup, float DeltaTime);
	void DecayAcceleration(float DeltaTime);
	void ApplyAcceleration() const;
	void ApplyWheelTorque(float DeltaTime);

	void ResetDragForces();

	void UpdatePhysicsBodyPositionData();

	void SendForceRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins);
	bool RayCastHit(FHitResult& RayCast, const FVector& LocalOrigin);
	void AddForcesOnRayCastHit(FHitResult& RayCast);
	void AddDragForceOnRayCastHit(float CompressionRatio);

	void SendWheelRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins);
	void ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin);

	void DrawRayCasts(RayCastGroup& RayCasts) const;
	void DrawRayCast(const FHitResult& RayCast) const;
	void DrawAcceleration() const;

	void AddBrakingDrag(float DeltaTime);
	void AddAirTimeAngularDrag();
	void AddAccelerationAngularDrag();
	void ApplyDragForces();

private:
	float TickCount{};

	bool TickReset{};

	FVector AccelerationSurfaceImpact{};
	FVector AccelerationSurfaceNormal{};

	float AccelerationForce{};
	float AccelerationRatio{};
	
	float TotalLinearDragForce{};
	float TotalAngularDragForce{};

	float WheelsGrounded{};

	float LinearBrakingDrag{};

	TArray<float> LinearDragForces{};
	TArray<float> AngularDragForces{};

	FTransform PhysicsBodyWorldTransform{};
	FTransform PhysicsBodyLocalTransform{};

	FVector PhysicsBodyLocation{};
	FVector PhysicsBodyUpVector{};
	FVector PhysicsBodyForwardVector{};
	FVector PhysicsBodyRightVector{};

	FRotator LocalCameraArmRotation{ CameraArmRotationOffset };
	FRotator WorldCameraArmRotation{};

	FHitResult FRForceRayCast{};
	FHitResult FLForceRayCast{};
	FHitResult BRForceRayCast{};
	FHitResult BLForceRayCast{};

	FHitResult FRWheelRayCast{};
	FHitResult FLWheelRayCast{};
	FHitResult BRWheelRayCast{};
	FHitResult BLWheelRayCast{};
	
	RayCastGroup ForceRayCasts{ FRForceRayCast, FLForceRayCast, BRForceRayCast, BLForceRayCast };
	RayCastGroup WheelRayCasts{ FRWheelRayCast, FLWheelRayCast, BRWheelRayCast, BLWheelRayCast };

};
