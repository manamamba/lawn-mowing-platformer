// Fill out your copyright notice in the Description page of Project Settings.


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
	void FloatPhysicsEnabledComponent(UPrimitiveComponent* Component);
	void TrackPhysicsEnabledComponentAcceleration(UPrimitiveComponent* Component, ChangeInVelocity& Velocity, float DeltaTime);
	double GetAcceleration(const FVector& Vector, ChangeInVelocity& Velocity, float DeltaTime);
	
	void RayCastAtDefaultPosition(UPrimitiveComponent* Component, const FVector& DefaultRayCastPosition, UStaticMeshComponent* WheelMesh, const FVector& DefaultMeshPosition);
	void ApplyDragToGroundedComponent(UPrimitiveComponent* Component);

public:
	void MoveCamera(const FInputActionValue& Value);


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

	const double MinArmPitch{ -89.0 };
	const double MaxArmPitch{ 5.0 };

	ChangeInVelocity PhysicsBodyVelocity{};

	double DragCompression{ 1.0 };
	int32 GroundedWheels{ 0 };
};
