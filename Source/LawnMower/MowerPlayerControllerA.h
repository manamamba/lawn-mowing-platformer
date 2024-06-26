// Derived APlayerController class AMowerPlayerControllerA by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MowerPlayerControllerA.generated.h"

class UUserWidget;


UCLASS()
class LAWNMOWER_API AMowerPlayerControllerA : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Widget) TSubclassOf<UUserWidget> PauseMenuClass{};

	UPROPERTY() UUserWidget* PauseMenu {};

protected:
	virtual void BeginPlay() override;

public:
	void DisplayPauseMenu();
	void HidePauseMenu();

};
