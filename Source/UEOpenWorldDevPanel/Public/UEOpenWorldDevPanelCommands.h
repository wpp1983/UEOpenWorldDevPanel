// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UEOpenWorldDevPanelStyle.h"

class FUEOpenWorldDevPanelCommands : public TCommands<FUEOpenWorldDevPanelCommands>
{
public:

	FUEOpenWorldDevPanelCommands()
		: TCommands<FUEOpenWorldDevPanelCommands>(TEXT("UEOpenWorldDevPanel"), NSLOCTEXT("Contexts", "UEOpenWorldDevPanel", "UEOpenWorldDevPanel Plugin"), NAME_None, FUEOpenWorldDevPanelStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
	TSharedPtr< FUICommandInfo> MoveCameraHere;
};