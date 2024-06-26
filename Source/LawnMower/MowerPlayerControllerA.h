// Derived APlayerController class AMowerPlayerControllerA by Cody Wheeler.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MowerPlayerControllerA.generated.h"

class UUserWidget;
class UWidget;
class UTextBlock;

enum PauseMenuVerticalNavigation : int {
	Resume = 1,
	Resolution,
	Volume,
	Restart,
	Exit
};

enum PauseMenuResolutionNavigation {
	Windowed = 1,
	FullScreen
};

enum PauseMenuVolumeNavigation {
	Zero = 1,
	Quarter,
	Half,
	ThreeQuarters,
	Full
};

enum PauseMenuConfirmationNavigation {
	Cancel = 1,
	Confirm
};


UCLASS()
class LAWNMOWER_API AMowerPlayerControllerA : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Widget) TSubclassOf<UUserWidget> PauseMenuClass{};

	UPROPERTY() UUserWidget* PauseMenu {};
	UPROPERTY() UTextBlock* SelectedTextBlock {};

	const FLinearColor SelectedColor{ 0.5f, 0.25f, 0.0f };

	const int MaxMainOptions{ 5 };
	const int MaxResolutionOptions{ 2 };
	const int MaxVolumeOptions{ 5 };
	const int MaxConfirmationOptions{ 2 };

protected:
	virtual void BeginPlay() override;

private:
	void UpdatePauseMenuTextArray();

public:
	void DisplayPauseMenu();
	void HidePauseMenu();
	void UpdateVerticalNavigation(const float NavigatingDirection);
	void UpdateHorizontalNavigation(const float NavigatingDirection);
	bool SelectOption();
	bool CancelOption();

private:
	void ResetHorizontalNavigation();
	template<typename T> T GetUpdatedNavigationOption(const T& Navigation, const float Direction, const int MaxOptions);
	void CheckNavigationBounds(int& Option, const int MaxOptions);

	void SelectResolutionOption();
	void SelectVolumeOption();
	void SelectRestartOption();
	void SelectExitOption();

	void UpdateSelectedPauseMenuOption();
	void UpdatePauseMenuOptionTextColor(const FName& TextBoxName);
	void PrintCurrentNavigationOption();

private:
	TArray<UTextBlock*> PauseMenuText{};

	bool bUsingVerticalNavigation{};
	bool bUsingResolutionNavigation{};
	bool bUsingVolumeNavigation{};
	bool bUsingRestartNavigation{};
	bool bUsingExitNavigation{};

	PauseMenuResolutionNavigation ResolutionSet{ Windowed };
	PauseMenuVolumeNavigation VolumeSet{ Full };

	PauseMenuVerticalNavigation VerticalNavigation{ Resume };
	PauseMenuResolutionNavigation ResolutionNavigation{ ResolutionSet };
	PauseMenuVolumeNavigation VolumeNavigation{ VolumeSet };
	PauseMenuConfirmationNavigation RestartNavigation{ Cancel };
	PauseMenuConfirmationNavigation ExitNavigation{ Cancel };

};