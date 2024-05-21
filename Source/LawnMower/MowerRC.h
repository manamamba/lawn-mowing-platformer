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

public:
	AMowerRC();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:
	void CreateAndAssignComponentSubObjects();
	void SetupComponentAttachments();
	void SetComponentProperties();
	void SetMeshComponentCollisionAndLocation(UStaticMeshComponent* Mesh, const FVector& Location);

	void AddInputMappingContextToLocalPlayerSubsystem();
	void SetPhysicsBodyMassProperties();

	void FloatMower();

	void ApplyAccelerationInputToGroundedWheels(RayCastGroup& RayCastGroup, float DeltaTime);
	void AddForceToGroundedWheel(FHitResult& RayCast, double Force);

	void UpdatePhysicsBodyPositionalData();
	void UpdatePhysicsBodyForceData(float DeltaTime);
	
	void SendForceRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins);
	bool RayCastHit(FHitResult& RayCast, const FVector& LocalOrigin);
	void AddForceOnRayCastHit(FHitResult& RayCast);
	void AddDragForceOnRayCastHit(double CompressionRatio);
	void SendWheelRayCasts(RayCastGroup& RayCastGroup, const LocalOrigins& LocalOrigins);
	void ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, FHitResult& RayCast, const FVector& LocalOrigin);
	
	void AddAngularDragForce();

	void DrawRayCasts(RayCastGroup& RayCasts);
	void DrawRayCast(FHitResult& RayCast);

	void ApplyDragForce();

public:
	void MoveCamera(const FInputActionValue& Value);
	void Accelerate(const FInputActionValue& Value);


private:
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
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction {};

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

	const double MinArmPitch{ -89.0 };
	const double MaxArmPitch{ 5.0 };
	const double RayCastLength{ 8.9 };
	const double WheelCount{ 4.0 };
	const double PhysicsBodyMass{ 30.0 };

	const double GravitationalAcceleration{ 980.0 };
	const double AntiGravitationalForce{ PhysicsBodyMass * GravitationalAcceleration };

	const double DragForceCompressionRatioMinimum{ 0.25 };
	const double MaxWheelDragForce{ 2.0 };

	const double AngularDragForceMultiplier{ 0.003 };

	const double AccelerationForceMaximum{ 15000.0 };
	const double AccelerationDecayRate{ 0.3 };
	const double AccelerationRatioMaximum{ 3.0 };

	FTransform PhysicsBodyTransform{};

	FVector PhysicsBodyLocation{};
	FVector PhysicsBodyUpVector{};
	FVector PhysicsBodyForwardVector{};
	FVector PhysicsBodyRightVector{};

	FVector PhysicsBodyVelocity{};
	FVector PhysicsBodyFinalVelocity{};
	FVector PhysicsBodyInitiallVelocity{};
	FVector PhysicsBodyLastTicklVelocity{};
	FVector PhysicsBodyVelocityNormal{};

	double PhysicsBodyChangeInVelocity{};
	double PhysicsBodyAcceleration{};
	double PhysicsBodyForce{};

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

	TArray<double> LinearDragForces{};
	TArray<double> AngularDragForces{};

	double AccelerationRatio{};
	double AcceleratingDirection{};

};
