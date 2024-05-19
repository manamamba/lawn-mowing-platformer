// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MowerRC.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;


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

public:
	void MoveCamera(const FInputActionValue& Value);

private:	
	void CreateAndAssignComponentSubObjects();
	void SetupComponentAttachments();
	void SetComponentProperties();
	void SetWheelProperties(UStaticMeshComponent* Wheel, FVector Location);
	void AddInputMappingContextToLocalPlayerSubsystem();

	void Float(float DeltaTime);

	double GetMowerAcceleration(float DeltaTime);
	void ApplyForceToGroundedMower(double Acceleration);
	

	void ApplyForceToWheelPosition(UStaticMeshComponent* Wheel, const FVector& WheelPosition);

private:	
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

	const double MinArmPitch{ -89.0 };
	const double MaxArmPitch{ 8.9 };
	const float BodyMass{ 30.0f };
	const FVector LocalFRWheelPosition{ 24.0, 24.0, -7.0 };
	const FVector LocalFLWheelPosition{ 24.0, -24.0, -7.0 };
	const FVector LocalBRWheelPosition{ -24.0, 24.0, -7.0 };
	const FVector LocalBLWheelPosition{ -24.0, -24.0, -7.0 };
	const double GravitationalAcceleration{ 980.0 };


	double MowerAcceleration{};

	FVector FinalVelocity{};
	FVector InitialVelocity{};
	FVector LastTickVelocity{};




};
