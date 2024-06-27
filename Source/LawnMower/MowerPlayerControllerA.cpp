// Derived APlayerController class AMowerPlayerControllerA by Cody Wheeler.


#include "MowerPlayerControllerA.h"

#include "AudioDevice.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "MowerGameModeA.h"


void AMowerPlayerControllerA::BeginPlay()
{
	Super::BeginPlay();

	PauseMenu = CreateWidget(this, PauseMenuClass);

	UpdatePauseMenuTextArray();
}

void AMowerPlayerControllerA::UpdatePauseMenuTextArray()
{
	TArray<UWidget*> PauseMenuCanvasPanelChildren{};

	if (UCanvasPanel * PauseMenuCanvasPanel{ Cast<UCanvasPanel>(PauseMenu->GetRootWidget()) })
	{
		PauseMenuCanvasPanelChildren = PauseMenuCanvasPanel->GetAllChildren();
	}

	for (auto& Child : PauseMenuCanvasPanelChildren) PauseMenuText.Add(Cast<UTextBlock>(Child));
}

void AMowerPlayerControllerA::DisplayPauseMenu()
{
	PauseMenu->AddToViewport();

	bUsingVerticalNavigation = true;

	UpdateSelectedPauseMenuOption();
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

	UpdateSelectedPauseMenuOption();
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

	UpdateSelectedPauseMenuOption();
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

		UpdateSelectedPauseMenuOption();
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
	if (ResolutionSet == ResolutionNavigation) return;
	
	ResolutionSet = ResolutionNavigation;

	GetWorld()->GetGameViewport()->HandleToggleFullscreenCommand();
}

void AMowerPlayerControllerA::SelectVolumeOption()
{
	VolumeSet = VolumeNavigation;

	float VolumeLevel{};

	switch (VolumeNavigation)
	{
	case Quarter:		VolumeLevel = 0.25f;	break;
	case Half:			VolumeLevel = 0.5f;		break;
	case ThreeQuarters: VolumeLevel = 0.75f;	break;
	case Full:			VolumeLevel = 1.0f;	
	}

	// TArray<USoundClass*> SoundClasses{};
	// TArray<USoundMix*> SoundMixes{};

	// TMap<USoundClass*, FSoundClassProperties> SoundClassMap{ GetWorld()->GetAudioDevice().GetAudioDevice()->GetSoundClassPropertyMap() };
	// TMap<USoundMix*, FSoundMixState> SoundMixMap{ GetWorld()->GetAudioDevice().GetAudioDevice()->GetSoundMixModifiers() };

	// SoundClassMap.GenerateKeyArray(SoundClasses);
	// SoundMixMap.GenerateKeyArray(SoundMixes);

	FAudioThread::RunCommandOnAudioThread([this, VolumeLevel]()
		{
			TArray<USoundClass*> SoundClasses{};
			TArray<USoundMix*> SoundMixes{};

			TMap<USoundMix*, FSoundMixState> SoundMixMap{ GetWorld()->GetAudioDevice().GetAudioDevice()->GetSoundMixModifiers() };

			SoundMixMap.GenerateKeyArray(SoundMixes);

			UGameplayStatics::SetSoundMixClassOverride(this, SoundMixes[0], SoundMixes[0]->SoundClassEffects[0].SoundClassObject, VolumeLevel);
			UGameplayStatics::PushSoundMixModifier(this, SoundMixes[0]);
		});

	// UGameplayStatics::SetSoundMixClassOverride(this, SoundMixes[0], SoundClasses[0], VolumeLevel);
	// UGameplayStatics::PushSoundMixModifier(this, SoundMixes[0]);
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

	UpdateSelectedPauseMenuOption();
	PrintCurrentNavigationOption();

	return false;
}

void AMowerPlayerControllerA::UpdateSelectedPauseMenuOption()
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
		case Half:			CurrentOption = TEXT("Half");		break;
		case ThreeQuarters: CurrentOption = TEXT("ThreeQuarters");		break;
		case Full:			CurrentOption = TEXT("Full");
		}
	}

	if (bUsingRestartNavigation)
	{
		switch (RestartNavigation)
		{
		case Cancel:	CurrentOption = TEXT("CancelRestart");	break;
		case Confirm:	CurrentOption = TEXT("ConfirmRestart");
		}
	}

	if (bUsingExitNavigation)
	{
		switch (ExitNavigation)
		{
		case Cancel:	CurrentOption = TEXT("CancelExit");		break;
		case Confirm:	CurrentOption = TEXT("ConfirmExit");
		}
	}

	UpdatePauseMenuOptionTextColor(CurrentOption);
}

void AMowerPlayerControllerA::UpdatePauseMenuOptionTextColor(const FName& TextBoxName)
{
	if (SelectedTextBlock) SelectedTextBlock->SetColorAndOpacity(FSlateColor{ FLinearColor::Black });
	
	for (auto& Text : PauseMenuText) if (Text->GetName() == TextBoxName) SelectedTextBlock = Text;

	if (!SelectedTextBlock) return;

	UE_LOG(LogTemp, Warning, TEXT("SelectedTextBlock %s"), *SelectedTextBlock->GetName());

	SelectedTextBlock->SetColorAndOpacity(FSlateColor{ SelectedColor });
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