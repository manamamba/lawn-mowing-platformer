// Derived APlayerController class AMowerPlayerControllerA by Cody Wheeler.


#include "MowerPlayerControllerA.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "MowerGameModeA.h"


void AMowerPlayerControllerA::BeginPlay()
{
	Super::BeginPlay();

	PauseMenu = CreateWidget(this, PauseMenuClass);
}

void AMowerPlayerControllerA::DisplayPauseMenu()
{
	PauseMenu->AddToViewport();

	bUsingVerticalNavigation = true;

	PrintCurrentNavigationOption();
}

void AMowerPlayerControllerA::HidePauseMenu()
{
	PauseMenu->RemoveFromParent();

	bUsingVerticalNavigation = false;
	VerticalNavigation = Resume;

	ResetHorizontalNavigation();
}

void AMowerPlayerControllerA::ResetHorizontalNavigation()
{
	bUsingResolutionNavigation = false;
	bUsingVolumeNavigation = false;
	bUsingRestartNavigation = false;
	bUsingExitNavigation = false;
	
	ResolutionNavigation = ResolutionSet;
	VolumeNavigation = VolumeSet;
	RestartNavigation = Cancel;
	ExitNavigation = Cancel;
}

void AMowerPlayerControllerA::UpdateVerticalNavigation(const float NavigatingDirection)
{
	if (!bUsingVerticalNavigation) return;

	VerticalNavigation = GetUpdatedNavigationOption(VerticalNavigation, NavigatingDirection, MaxMainOptions);

	PrintCurrentNavigationOption();
}

template<typename T> T AMowerPlayerControllerA::GetUpdatedNavigationOption(const T& Navigation, const float Direction, const int MaxOptions)
{
	int CurrentOption = { Navigation };

	CurrentOption += static_cast<int>(Direction);

	CheckNavigationBounds(CurrentOption, MaxOptions);

	return static_cast<T>(CurrentOption);
}

void AMowerPlayerControllerA::CheckNavigationBounds(int& Option, const int MaxOptions)
{
	if (Option > MaxOptions) Option = 1;
	if (Option < 1) Option = MaxOptions;
}

void AMowerPlayerControllerA::UpdateHorizontalNavigation(const float NavigatingDirection)
{
	if (bUsingVerticalNavigation) return;

	if (bUsingResolutionNavigation) ResolutionNavigation = GetUpdatedNavigationOption(ResolutionNavigation, NavigatingDirection, MaxResolutionOptions);

	if (bUsingVolumeNavigation) VolumeNavigation = GetUpdatedNavigationOption(VolumeNavigation, NavigatingDirection, MaxVolumeOptions);

	if (bUsingRestartNavigation) RestartNavigation = GetUpdatedNavigationOption(RestartNavigation, NavigatingDirection, MaxConfirmationOptions);

	if (bUsingExitNavigation) ExitNavigation = GetUpdatedNavigationOption(ExitNavigation, NavigatingDirection, MaxConfirmationOptions);

	PrintCurrentNavigationOption();
}

bool AMowerPlayerControllerA::SelectOption()
{
	if (bUsingVerticalNavigation)
	{
		switch (VerticalNavigation)
		{
		case Resolution:	bUsingResolutionNavigation = true;		break;
		case Volume:		bUsingVolumeNavigation = true;			break;
		case Restart:		bUsingRestartNavigation = true;			break;
		case Exit:			bUsingExitNavigation = true;			break;
		default:			return true;
		}

		bUsingVerticalNavigation = false;

		PrintCurrentNavigationOption();

		return false;
	}

	if (bUsingResolutionNavigation) SelectResolutionOption();
	if (bUsingVolumeNavigation) SelectVolumeOption();
	if (bUsingRestartNavigation) SelectRestartOption();
	if (bUsingExitNavigation) SelectExitOption();

	return false;
}

void AMowerPlayerControllerA::SelectResolutionOption()
{
	ResolutionSet = ResolutionNavigation;
	
	switch (ResolutionNavigation)
	{
	case Windowed: UE_LOG(LogTemp, Warning, TEXT("Windowed Selected"));			break;
	case FullScreen: UE_LOG(LogTemp, Warning, TEXT("FullScreen Selected"));
	}
}

void AMowerPlayerControllerA::SelectVolumeOption()
{
	VolumeSet = VolumeNavigation;
	
	switch (VolumeNavigation)
	{
	case Zero: UE_LOG(LogTemp, Warning, TEXT("Zero Selected"));						break;
	case Quarter: UE_LOG(LogTemp, Warning, TEXT("Quarter Selected"));				break;
	case Half: UE_LOG(LogTemp, Warning, TEXT("Half Selected"));						break;
	case ThreeQuarters: UE_LOG(LogTemp, Warning, TEXT("ThreeQuarters Selected"));	break;
	case Full: UE_LOG(LogTemp, Warning, TEXT("Full Selected"));
	}
}

void AMowerPlayerControllerA::SelectRestartOption()
{
	if (RestartNavigation == Cancel)
	{
		CancelOption();
		return;
	}

	if (AMowerGameModeA * GameMode{ Cast<AMowerGameModeA>(UGameplayStatics::GetGameMode(this)) }) GameMode->RestartGame();
}

void AMowerPlayerControllerA::SelectExitOption()
{
	if (ExitNavigation == Cancel)
	{
		CancelOption();
		return;
	}

	UKismetSystemLibrary::QuitGame(this, UGameplayStatics::GetPlayerController(this, 0), EQuitPreference::Type::Quit, true);
}

bool AMowerPlayerControllerA::CancelOption()
{
	if (bUsingVerticalNavigation) return true;

	bUsingVerticalNavigation = true;

	ResetHorizontalNavigation();

	PrintCurrentNavigationOption();

	return false;
}

void AMowerPlayerControllerA::PrintCurrentNavigationOption()
{
	FName CurrentOption{};
	
	if (bUsingVerticalNavigation)
	{
		switch (VerticalNavigation)
		{
		case Resume: CurrentOption = TEXT("Resume");			break;
		case Resolution: CurrentOption = TEXT("Resolution");	break;
		case Volume: CurrentOption = TEXT("Volume");			break;
		case Restart: CurrentOption = TEXT("Restart");			break;
		case Exit: CurrentOption = TEXT("Exit");
		}
	}

	if (bUsingResolutionNavigation)
	{
		switch (ResolutionNavigation)
		{
		case Windowed: CurrentOption = TEXT("Windowed");		break;
		case FullScreen: CurrentOption = TEXT("FullScreen");
		}
	}

	if (bUsingVolumeNavigation)
	{
		switch (VolumeNavigation)
		{
		case Zero:			CurrentOption = TEXT("Zero");			break;
		case Quarter:		CurrentOption = TEXT("Quarter");		break;
		case Half:			CurrentOption = TEXT("Half");			break;
		case ThreeQuarters: CurrentOption = TEXT("ThreeQuarters");	break;
		case Full:			CurrentOption = TEXT("Full");
		}
	}

	if (bUsingRestartNavigation)
	{
		switch (RestartNavigation)
		{
		case Cancel:	CurrentOption = TEXT("Cancel Restart");		break;
		case Confirm:	CurrentOption = TEXT("Confirm Restart");
		}
	}

	if (bUsingExitNavigation)
	{
		switch (ExitNavigation)
		{
		case Cancel:	CurrentOption = TEXT("Cancel Exit");		break;
		case Confirm:	CurrentOption = TEXT("Confirm Exit");
		}
	}

	UE_LOG(LogTemp, Warning, TEXT(" "));
	UE_LOG(LogTemp, Warning, TEXT("%s"), *CurrentOption.ToString());
}