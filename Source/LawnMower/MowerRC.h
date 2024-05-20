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


struct ChangeInVelocity {
	FVector Final{};
	FVector Initial{};
	FVector LastTick{};
};

struct RayCast {
	FVector RayCastStart{};
	FHitResult Hit{};
	double CompressionRatio{};
	double DragForce{};
};

struct AllForceRayCasts {
	RayCast FR{};
	RayCast FL{};
	RayCast BR{};
	RayCast BL{};
};

struct WheelCast {
	FVector WheelCastStart{};
	FHitResult Hit{};
};

struct AllWheelRayCasts {
	WheelCast FR{};
	WheelCast FL{};
	WheelCast BR{};
	WheelCast BL{};
};

struct LocalStarts {
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
	void SetMeshComponentCollisionAndDefaultLocation(UStaticMeshComponent* Mesh, const FVector& Location);
	void AddInputMappingContextToLocalPlayerSubsystem();

	void FloatMower();
	void TrackMowerForceDirection(float DeltaTime);
	double GetAcceleration(const FVector& Vector, ChangeInVelocity& Velocity, float DeltaTime);

	void SetPhysicsBodyTickData();
	void SendForceRayCasts(AllForceRayCasts& SentForceRayCasts, const LocalStarts& SentForceRayCastStarts);
	bool RayCastHit(RayCast& RayCast, const FVector& DefaultRayCastPosition);
	void ApplyForceOnRayCast(RayCast& RayCast);
	void SendWheelRayCasts(AllWheelRayCasts& SentWheelRayCasts, const LocalStarts& SentWheelRayCastStarts);
	void ApplySuspensionOnWheel(UStaticMeshComponent* Wheel, WheelCast& WheelCast, const FVector& DefaultWheelPosition);
	
	void DrawForceAndWheelRayCasts(AllForceRayCasts& SentForceRayCasts, AllWheelRayCasts& SentWheelRayCasts);
	void DrawRayCastData(FVector RayCastStart, FHitResult Hit);

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

	const LocalStarts ForceRayCastStarts{ FRRayCastPosition, FLRayCastPosition, BRRayCastPosition, BLRayCastPosition };
	const LocalStarts WheelRayCastStarts{ FRWheelPosition, FLWheelPosition, BRWheelPosition, BLWheelPosition };

	const double MinArmPitch{ -89.0 };
	const double MaxArmPitch{ 5.0 };
	const double RayCastLength{ 8.9 };
	const double WheelCount{ 4.0 };
	const double Mass{ 16.850502 };
	const double GravitationalAcceleration{ 980.0 };
	const double AntiGravitationalForce{ Mass * GravitationalAcceleration };

	const double DragForceCompressionRatioMinimum{ 0.25 };
	const double MaxWheelDragForce{ 2.0 };
	const double LinearDragForceMultiplier{ 1.0 };
	const double AngularDragForceMultiplier{ 1.0 };

	ChangeInVelocity MowerVelocity{};

	FTransform PhysicsBodyTransform{};
	FVector PhysicsBodyUpVector{};
	FVector PhysicsBodyForwardVector{};
	FVector PhysicsBodyRightVector{};

	RayCast FRRayCast{};
	RayCast FLRayCast{};
	RayCast BRRayCast{};
	RayCast BLRayCast{};

	AllForceRayCasts ForceRayCasts{ FRRayCast, FLRayCast, BRRayCast, BLRayCast };

	WheelCast FRWheelCast{};
	WheelCast FLWheelCast{};
	WheelCast BRWheelCast{};
	WheelCast BLWheelCast{};

	AllWheelRayCasts WheelRayCasts{ FRWheelCast, FLWheelCast, BRWheelCast, BLWheelCast };

	TArray<double> DragForces{};



	const double MaxWheelDrag{ 1.5 };
	const double DragCompressionMinimum{ 0.25 };

	double DragCompression{ 1.0 };
	int32 GroundedWheels{ 0 };

};
