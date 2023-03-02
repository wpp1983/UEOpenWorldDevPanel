// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEOpenWorldDevPanelCommands.h"

#define LOCTEXT_NAMESPACE "FUEOpenWorldDevPanelModule"

void FUEOpenWorldDevPanelCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "UEOpenWorldDevPanel", "Bring up UEOpenWorldDevPanel window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CollectPostProcessVolume, "CollectPostProcessVolume", "Collect APostProcessVolume in current map", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
