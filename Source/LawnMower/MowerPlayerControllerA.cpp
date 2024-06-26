// Derived APlayerController class AMowerPlayerControllerA by Cody Wheeler.


#include "MowerPlayerControllerA.h"
#include "Blueprint/UserWidget.h"


void AMowerPlayerControllerA::BeginPlay()
{
	Super::BeginPlay();

	PauseMenu = CreateWidget(this, PauseMenuClass);
}

void AMowerPlayerControllerA::DisplayPauseMenu()
{
	PauseMenu->AddToViewport();

	UE_LOG(LogTemp, Warning, TEXT("DisplayPauseMenu Called!"));
}

void AMowerPlayerControllerA::HidePauseMenu()
{
	PauseMenu->RemoveFromParent();

	UE_LOG(LogTemp, Warning, TEXT("HidePauseMenu Called!"));
}