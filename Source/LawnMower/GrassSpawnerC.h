// Derived AActor class AGrassSpawnerB by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrassSpawnerC.generated.h"

class UBoxComponent;
class AGrassD;


UCLASS()
class LAWNMOWER_API AGrassSpawnerC : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) USceneComponent* Root {};
	UPROPERTY(EditAnywhere) UBoxComponent* Collider {};
	UPROPERTY(EditAnywhere) TSubclassOf<AGrassD> GrassClass{};

public:	
	AGrassSpawnerC();

private:
	void SetComponentProperties();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION() void ActivateSpawner(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);


private:
	bool bSpawnerActivated{};


};
