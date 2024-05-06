// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MowerKart.generated.h"


class UBoxComponent;
class UArrowComponent;
class USpringArmComponent;
class UCameraComponent;
class AMowerWheel;

UCLASS()
class LAWNMOWER_API AMowerKart : public APawn
{
	GENERATED_BODY()

public:
	AMowerKart();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

public:	
	

private:


private:
	UPROPERTY(EditAnywhere, Category = Component) UStaticMeshComponent* Body {};
	UPROPERTY(EditAnywhere, Category = Component) UStaticMeshComponent* FRWheel{};
	UPROPERTY(EditAnywhere, Category = Component) UStaticMeshComponent* FLWheel{};
	UPROPERTY(EditAnywhere, Category = Component) UStaticMeshComponent* BRWheel{};
	UPROPERTY(EditAnywhere, Category = Component) UStaticMeshComponent* BLWheel{};
	UPROPERTY(EditAnywhere, Category = Component) UBoxComponent* BoxCollision{};
	UPROPERTY(EditAnywhere, Category = Component) UArrowComponent* Arrow{};
	UPROPERTY(EditAnywhere, Category = Component) USpringArmComponent* SpringArm{};
	UPROPERTY(EditAnywhere, Category = Component) UCameraComponent* Camera{};


};
