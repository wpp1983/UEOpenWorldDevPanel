// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEOpenWorldDevPanelCommands.h"

#define LOCTEXT_NAMESPACE "FUEOpenWorldDevPanelModule"

void FUEOpenWorldDevPanelCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "UEOpenWorldDevPanel", "Bring up UEOpenWorldDevPanel window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(MoveCameraHere, "Move Camera Here", "Move the camera to the selected position.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DelecteSelected, "Delecte Selected", "Delecte Selected Icons.", EUserInterfaceActionType::Button, FInputChord());

}

#undef LOCTEXT_NAMESPACE
