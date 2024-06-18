// Derived APawn class AMowerB by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MowerB.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UNiagaraComponent;
class UAudioComponent;
struct FInputActionValue;


UCLASS()
class LAWNMOWER_API AMowerB : public APawn
{
	GENERATED_BODY()

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

	UPROPERTY(EditDefaultsOnly, Category = Component) UBoxComponent* PhysicsBody {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UBoxComponent* Collider {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Body {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* Blade {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* FrWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* FlWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* BrWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UStaticMeshComponent* BlWheel {};
	UPROPERTY(EditDefaultsOnly, Category = Component) USpringArmComponent* CameraArm {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UCameraComponent* Camera {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UNiagaraComponent* Emitter {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UAudioComponent* EngineAudio {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UAudioComponent* MovementAudio {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UAudioComponent* JumpAudio {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UAudioComponent* CrashAudio {};
	UPROPERTY(EditDefaultsOnly, Category = Component) UAudioComponent* CutAudio {};

	UPROPERTY(EditDefaultsOnly, Category = Input) UInputMappingContext* InputMappingContext {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* MoveCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* ResetCameraInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* PitchInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* BrakeInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* SteerInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* DriftInputAction {};
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* JumpInputAction {};

	const float GravitationalAcceleration{ 980.0f };
	const float PhysicsBodyMass{ 30.0f };
	const float PhysicsBodyAntiGravitationalForce{ PhysicsBodyMass * GravitationalAcceleration };
	const double PhysicsBodyCenterOfMassOffset{ PhysicsBodyMass / 2.0 };
	const double PhysicsBodyRespawnHeight{ -1000.0 };
	const FVector PhysicsBodyCenterOfMass{ 0.0, 0.0, -PhysicsBodyCenterOfMassOffset };
	const FVector PhysicsBodyDimensions{ 30.5, 20.0, 9.0 };
	const FVector ColliderDimensions{ 36.0, 30.0, 4.0 };

	const FVector ColliderPosition{ 0.0, 0.0, -13.0 };
	const FVector BodyPosition{ -0.3, 0.0, -1.0 };
	const FVector BladePosition{ -0.25, 0.0, -7.0 };
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

	const float AccelerationForceMaximum{ 10000.0f };
	const float AccelerationRatioMaximum{ 3.0f };
	const float AccelerationRatioRate{ 1.25f };
	const float AccelerationDecayRate{ 0.5f };
	const float AccelerationRatioMinimumWhileBraking{ 0.15f };
	const float AccelerationBrakingRate{ 1.75f };
	const float AccelerationBrakingDecayRate{ 0.5f };
	const float AccelerationMaxLinearDrag{ 8.01 };

	const double SteeringForceMultiplier{ 200.0 };
	const double SteeringForceOnSlopeRate{ 0.2 };

	const double DriftingForceMaximum{ 2000.0 };
	const double DriftingForcePositionOffset{ 25.0 };
	const float DriftingRatioMaximum{ 3.0f };
	const float DriftingRate{ 3.0f };
	const float DriftingDecayRate{ 3.0f };

	const float JumpReadyForce{ 25000.0 };
	const float JumpReadyRatioMaximum{ 1.0f };
	const float JumpReadyRate{ 3.0f };

	const double JumpingForceMaximum{ 275000.0 };
	const float JumpingRatioMaximum{ 1.0f };
	const float JumpingRate{ 5.0f };

	const double AirTimeAntiGravitationalForce{ PhysicsBodyMass * 245.0 };
	const double AirTimePitchTorque{ 30000000.0 };
	const double AirTimeRollTorque{ 15000000.0 };
	const float AirTimeRatioMaxium{ 3.0f };
	const float AirTimeRatioIncreaseRate{ 2.0f };
	const float AirTimeMinimum{ 0.6f };

	const double StalledRollTorqueMultiplier{ 2.5 };

	const float BrakingLinearDragRate{ 0.25f };
	const float BrakingLinearDragLimit{ 50.0f };
	const float AcceleratingAngularDragRate{ 0.000005f };
	const float AirTimeAngularDrag{ 5.0f };

	const double WheelAcceleratingPitchRate{ -1200.0 };
	const double WheelDriftingPitchRate{ -300.0 };
	const double WheelOnSlopePitchRate{ 2.0 };
	const double WheelSteeringMaximum{ 8.0 };
	const float WheelPitchMinimumWhileDrifting{ 0.001f };
	const float WheelSteeringRatioMaximum{ 3.0f };
	const float WheelSteeringRate{ 8.0f };
	const float WheelSteeringDecayRate{ 12.0f };

	const float EmitterTimeMaximum{ 0.25f };
	const double BladeRotationRate{ 720.0 };

	const float EnginePitchMultiplierDefault{ 0.75f };
	const float EnginePitchAccelerationDivisor{ 12.0f };
	const float CrashAudioTimerReady{ 0.26f };
	const float CutAudioTimerMaximum{ 0.08f };

public:
	AMowerB();

private:
	void CreateAndAssignComponentSubObjects();
	void SetupComponentAttachments();
	void SetComponentProperties();
	void SetMeshComponentCollisionAndLocation(UStaticMeshComponent* Mesh, const FVector& Location);

	UFUNCTION() void PlayCrashAudio(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION() void StartEmitter(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

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
	void Jump(const FInputActionValue& Value);

public:
	void Float() const;
	float GetPhysicsBodyMass() const;
	class UBoxComponent* GetPhysicsBody();
	void SetNewRespawnLocation(const FVector& NewLocation);

private:
	FVector2D RotatingCameraDirection{};
	bool bCameraReset{};
	float AcceleratingDirection{};
	float PitchingDirection{};
	float Braking{};
	float Steering{};
	float Drifting{};
	float Jumping{};

public:
	virtual void Tick(float DeltaTime) override;

private:
	void GetTickTime();

	void UpdateTransforms();

	void Respawn();

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
	void UpdateJumpReadyRatio(const float DeltaTime);
	void UpdateJumpingRatio(const float DeltaTime);
	void DecayRatio(float& Ratio, const float DecayRate, const float DeltaTime);
	void LimitRatio(float& Ratio, const float RatioMaximum);
	void UpdateAcceleratingDirection();

	void ApplyAccelerationForce();
	void ApplySteeringTorque();
	void ApplyDriftingForce();
	void ApplyJumpingForce();

	void UpdateAirTimeRatio(const float DeltaTime);

	void ApplyAirTimeAntiGravitationalForce();
	void ApplyAirTimePitchTorque();
	void ApplyAirTimeRollTorque();

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
	void ApplyBladeRotation(const float DeltaTime);
	void ApplyMowerVibration(const float DeltaTime);

	void UpdateEmitterTime(const float DeltaTime);

	void UpdateEngineAudioPitch();
	void UpdateMovementAudioVolumeAndPitch();
	void PlayJumpAudio();
	void UpdateCrashAudioTimer(const float DeltaTime);
	void UpdateCutAudioTimer(const float DeltaTime);
	void PlayCutAudio();

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
	FVector RespawnLocation{};

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
	double HoverSpeedMultiplier{};

	bool bMovingByAccumulatedAcceleration{};
	bool bAccelerating{};
	bool bAcceleratingLastTick{};
	bool bSteering{};
	bool bLastAccelerationWasForward{};

	FVector AccelerationSurfaceImpact{};
	FVector AccelerationSurfaceNormal{};
	FVector DriftingForcePosition{};
	float AccelerationForce{};
	float AccelerationRatio{};
	float DriftingRatio{};

	float JumpReadyRatio{};
	bool bJumpReady{};

	FVector JumpingForceDirection{};
	float JumpingRatio{};
	bool bStartedJumping{};

	float AirTimeRatio{};
	bool bAirTimeMinimumExceeded{};

	TArray<float> LinearDragArray{};
	TArray<float> AngularDragArray{};
	float TotalLinearDragLastTick{};
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

	float EmitterTime{};
	bool bEmitterActivated{};
	mutable FRotator LocalBladeRotation{};

	FString LastActorCrash{};
	float CrashAudioTimer{};
	float CutAudioTimer{};


	float TickCount{};
	bool bTickReset{};
};