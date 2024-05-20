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
	








	void RayCastAtDefaultPosition(UPrimitiveComponent* Component, const FVector& DefaultRayCastPosition, UStaticMeshComponent* WheelMesh, const FVector& DefaultMeshPosition);
	void ApplyDragToGroundedMower();

public:
	void MoveCamera(const FInputActionValue& Value);
	void Accelerate(const FInputActionValue& Value);


private:	
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
	UPROPERTY(EditDefaultsOnly, Category = Input) UInputAction* AccelerateInputAction{};

	const FVector PhysicsBodyDimensions{ 30.5, 20.0, 9.0 };
	const FVector DefaultBodyPosition{ -0.3, 0.0, -1.0 };
	const FVector DefaultHandlePosition{ -23.3, 0.0, 0.0 };
	const FVector DefaultFRWheelPosition{ 26.0, 24.0, -9.0 };
	const FVector DefaultFLWheelPosition{ 26.0, -24.0, -9.0 };
	const FVector DefaultBRWheelPosition{ -26.0, 24.0, -9.0 };
	const FVector DefaultBLWheelPosition{ -26.0, -24.0, -9.0 };

	const FVector FRRayCastDefaultPosition{ 25.0, 15.0, -9.0 };
	const FVector FLRayCastDefaultPosition{ 25.0, -15.0, -9.0 };
	const FVector BRRayCastDefaultPosition{ -25.0, 15.0, -9.0 };
	const FVector BLRayCastDefaultPosition{ -25.0, -15.0, -9.0 };

	static constexpr double MinArmPitch{ -89.0 };
	static constexpr double MaxArmPitch{ 5.0 };
	static constexpr double RayCastLength{ 8.9 };
	static constexpr double WheelCount{ 4.0 };
	static constexpr double Mass{ 16.850502 };
	static constexpr double GravitationalAcceleration{ 980.0 };
	static constexpr double AntiGravitationalForce{ Mass * GravitationalAcceleration };
	static constexpr double DragCompressionMinimum{ 0.25 };
	static constexpr double MaxWheelDrag{ 2.0 };








	ChangeInVelocity MowerVelocity{};

	double DragCompression{ 1.0 };
	int32 GroundedWheels{ 0 };

};
